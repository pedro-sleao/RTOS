import serial
import threading
import logging
import time
import sys
import os.path


#
# Inicialização e definições
#

logging.basicConfig(level=logging.INFO)

if len(sys.argv) > 1:
    serial_name = sys.argv[1]
else:
    serial_name = "/dev/ttyUSB0" # Com o arduino, use /dev/ttyUSB0

if not os.path.exists(serial_name):
    logging.error(f"O arquivo {serial_name} não existe. Verifique se o arduino está conectado corretamente ou se a serial está devidamente configurada.")
    sys.exit(-1)

SYN, ESC, XON, XOFF = 0x7e, 0x7d, 0x11, 0x13
EV_SYN, EV_ESC, EV_XON, EV_XOFF, EV_CHAR, \
    EV_READ, EV_WRITE, EV_TX_READY, \
    EV_RX_XOFF, EV_RX_XON, EV_TX_XOFF, EV_TX_XON, \
    EV_WRITE = range(13)

#
# Funções implementando a escrita de bytes
#

g_recvd = bytearray()
g_max_n_recvd, g_n_recvd = 0, 0
g_rx_state = "S0"
g_is_xoff_recvd = False
g_is_xoff_sent = False
g_syn_on_close = True
g_read_end_ev = threading.Event()
g_write_end_ev = threading.Event()
g_tx_char_ev = threading.Event()
g_xoff_active_ev = threading.Event()
g_stop_all_ev = threading.Event()
g_rx_sm_mtx = threading.Lock()
g_data = 0
g_tx_data = 0
g_2nd_char = 0
g_tx_state = "T0a"
g_tx_sm_mtx = threading.Lock()
g_idle_buffer = bytearray()

def read_daemon(ser):
    global g_data

    logging.debug("Entrando na função read_daemon()")
    while not g_stop_all_ev.is_set():
        x = ser.read()          # read one byte
        if not x:
            continue
        logging.debug(f"Recebido byte {x[0]}")
        g_data = x[0]
        if g_data == SYN:
            x = EV_SYN
        elif g_data == ESC:
            x = EV_ESC
        elif g_data == XON:
            x = EV_XON
        elif g_data == XOFF:
            x = EV_XOFF
        else:
            x = EV_CHAR

        rx_process_event(x)

    logging.debug("Saindo   da função read_daemon()")

def rx_process_event(ev):
    global g_rx_state, g_recvd, g_max_n_recvd, g_n_recvd, g_read_reqtd, g_rx_sm_mtx, g_data, g_idle_buffer

    logging.debug("Entrando na função rx_process_event()")

    logging.debug(f"Evento recebido = {ev} no estado {g_rx_state}")

    with g_rx_sm_mtx:
        if g_rx_state == "S0":
            if ev == EV_ESC:
                g_rx_state = "S1"
            elif ev == EV_CHAR or ev == EV_XOFF or ev == EV_XON:
                g_idle_buffer.append(g_data)
            elif ev == EV_READ:
                g_rx_state = "S2"
                if g_idle_buffer:
                    if len(g_idle_buffer) < g_max_n_recvd:
                        g_recvd = g_idle_buffer
                    else:
                        g_recvd = g_idle_buffer[1-g_max_n_recvd:]
                        g_n_recvd = g_max_n_recvd + 1
                        g_rx_state = "S0"
                        g_read_end_ev.set()
                    g_idle_buffer = bytearray()
        elif g_rx_state == "S1":
            if ev == EV_XON:
                g_xoff_active_ev.clear()
                logging.debug(f"Recebido um XON no estado S1")
                tx_process_event(EV_RX_XON)
                g_rx_state = "S0"
            elif ev == EV_XOFF:
                g_xoff_active_ev.set()
                logging.debug(f"Recebido um XOFF no estado S1")
                tx_process_event(EV_RX_XOFF)
                g_rx_state = "S0"
            elif ev == EV_READ:
                g_rx_state = "S4"
        elif g_rx_state == "S4":
            if ev == EV_XON:
                g_xoff_active_ev.clear()
                logging.debug(f"Recebido um XON no estado S4")
                tx_process_event(EV_RX_XON)
                g_rx_state = "S2"
            elif ev == EV_XOFF:
                g_xoff_active_ev.set()
                logging.debug(f"Recebido um XOFF no estado S4")
                tx_process_event(EV_RX_XOFF)
                g_rx_state = "S2"
        elif g_rx_state == "S2":
            if ev == EV_ESC:
                g_rx_state = "S3"
            elif ev == EV_SYN:
                logging.debug("Recebido um SYN")
                g_n_recvd = len(g_recvd)
                g_rx_state = "S0"
                g_read_end_ev.set()
            elif ev == EV_CHAR or ev == EV_XOFF or ev == EV_XON:
                g_recvd.append(g_data)
                if len(g_recvd) == g_max_n_recvd:
                    g_n_recvd = g_max_n_recvd + 1
                    g_rx_state = "S0"
                    g_read_end_ev.set()
        elif g_rx_state == "S3":
            g_rx_state = "S2"
            if ev == EV_XON:
                g_xoff_active_ev.clear()
                logging.debug(f"Recebido um XON no estado S3")
            elif ev == EV_XOFF:
                g_xoff_active_ev.set()
                logging.debug(f"Recebido um XOFF no estado S3")
            elif ev == EV_SYN or ev == EV_ESC:
                g_recvd.append(g_data)
        else:
            logging.warning(f"Estado desconhecido ({g_rx_state})")
            g_rx_state = "S0"

    logging.debug("Saindo  da função rx_process_event()")

#
# Funções implementando a escrita de bytes
#

def write_daemon(ser):
    global g_tx_data, g_tx_char_ev, g_stop_all_ev

    logging.debug("Entrando na função write_daemon()")
    while not g_stop_all_ev.is_set():
        tx_process_event(EV_TX_READY)
        g_tx_char_ev.wait()
        g_tx_char_ev.clear()
        logging.debug(f"Writing {g_tx_data}")
        ser.write([g_tx_data])
        ser.flush()

    logging.debug("Saindo   da função write_daemon()")

def tx_process_event(ev):
    global g_tx_state, g_is_xoff_sent
    global g_is_xoff_recvd, g_2nd_char, g_syn_on_close
    global g_write_end_ev, g_n_trmtd, g_max_n_trmtd
    global g_tx_data

    logging.debug("Entrando na função tx_process_event()")

    if ev == EV_TX_XOFF:
        logging.info(f"Recebido evento TX_OFF no estado {g_tx_state}")
    elif ev == EV_TX_XON:
        logging.info(f"Recebido evento TX_OFF no estado {g_tx_state}")

    with g_tx_sm_mtx:
        if g_tx_state == "T0a":
            if ev == EV_RX_XOFF:
                g_tx_state = "T1a"
                g_is_xoff_recvd = True
            elif ev == EV_TX_XOFF:
                g_tx_state = "T2b"
                g_is_xoff_recvd = False
                g_is_xoff_sent = True
                g_tx_data = ESC
                g_2nd_char = XOFF
                g_tx_char_ev.set()
            elif ev == EV_TX_XON:
                g_tx_state = "T2b"
                g_is_xoff_recvd = False
                g_is_xoff_sent = False
                g_tx_data = ESC
                g_2nd_char = XON
                g_tx_char_ev.set()
            elif ev == EV_WRITE:
                ch = g_trmtd[g_n_trmtd]
                g_n_trmtd += 1
                g_tx_state, g_tx_data, g_2nd_char = ("T4", ESC, ch) \
                    if ch == SYN or ch == ESC else ("T3", ch, 0)
                g_tx_char_ev.set()
        elif g_tx_state == "T0b":
            if ev == EV_TX_READY:
                g_tx_state = "T0a"
                g_write_end_ev.set()
            elif ev == EV_RX_XOFF:
                g_tx_state = "T1b"
                g_is_xoff_recvd = True
            elif ev == EV_TX_XOFF:
                g_tx_state = "T2c"
                g_is_xoff_sent = True
            elif ev == EV_TX_XON:
                g_tx_state = "T2c"
                g_is_xoff_sent = False
        elif g_tx_state == "T0c":
            if ev == EV_TX_READY:
                g_tx_data = g_2nd_char
                g_tx_char_ev.set()
            elif ev == EV_RX_XOFF:
                g_tx_state = "T1c"
                g_is_xoff_recvd = True
            elif ev == EV_TX_XOFF:
                g_tx_state = "T2d"
                g_is_xoff_sent = True
            elif ev == EV_TX_XON:
                g_tx_state = "T2d"
                g_is_xoff_sent = False
        elif g_tx_state == "T1a":
            if ev == EV_RX_XON:
                g_tx_state = "T0a"
                g_is_xoff_recvd = False
            elif ev == EV_TX_XOFF:
                g_tx_state = "T2b"
                g_tx_data = ESC
                g_2nd_char = XOFF
                g_is_xoff_sent = True
                g_tx_char_ev.set()
            elif ev == EV_TX_XON:
                g_tx_state = "T2b"
                g_tx_data = ESC
                g_2nd_char = XON
                g_is_xoff_sent = False
                g_tx_char_ev.set()
            elif ev == EV_WRITE:
                g_write_end_ev.set()
        elif g_tx_state == "T1b":
            if ev == EV_TX_READY:
                g_tx_state = "T1a"
                g_write_end_ev.set()
            elif ev == EV_RX_XON:
                g_tx_state = "T0b"
                g_is_xoff_recvd = True
            elif ev == EV_TX_XOFF:
                g_tx_state = "T2c"
                g_is_xoff_sent = True
            elif ev == EV_TX_XON:
                g_tx_state = "T2c"
                g_is_xoff_sent = False
        elif g_tx_state == "T1c":
            if ev == EV_TX_READY:
                g_tx_state = "T1b"
                g_tx_data = g_2nd_char
                g_tx_char_ev.set()
            elif ev == EV_RX_XON:
                g_tx_state = "T0c"
                g_is_xoff_recvd = True
            elif ev == EV_TX_XOFF:
                g_tx_state = "T2d"
                g_is_xoff_sent = True
            elif ev == EV_TX_XON:
                g_tx_state = "T2d"
                g_is_xoff_sent = False
        elif g_tx_state == "T2a":
            if ev == EV_TX_READY:
                g_tx_state = "T1a" if g_is_xoff_recvd else "T0a"
                g_write_end_ev.set()
            elif ev == EV_RX_XON:
                g_is_xoff_recvd = False
            elif ev == EV_RX_XOFF:
                g_is_xoff_recvd = True
        elif g_tx_state == "T2b":
            if ev == EV_TX_READY:
                g_tx_state = "T2a"
                g_tx_data = g_2nd_char
                g_tx_char_ev.set()
            elif ev == EV_RX_XON:
                g_is_xoff_recvd = False
            elif ev == EV_RX_XOFF:
                g_is_xoff_recvd = True
        elif g_tx_state == "T2c":
            if ev == EV_TX_READY:
                g_tx_state = "T2b"
                g_tx_data = ESC
                g_2nd_char = XOFF if g_is_xoff_sent else XON
                g_tx_char_ev.set()
            elif ev == EV_TX_XON and g_is_xoff_sent:
                g_tx_state = "T1b" if g_is_xoff_recvd else "T0b"
            elif ev == EV_TX_XOFF and not g_is_xoff_sent:
                g_tx_state = "T1b" if g_is_xoff_recvd else "T0b"
            elif ev == EV_RX_XON:
                g_is_xoff_recvd = False
            elif ev == EV_RX_XOFF:
                g_is_xoff_recvd = True
        elif g_tx_state == "T2d":
            if ev == EV_TX_READY:
                g_tx_state = "T2c"
                g_tx_data = g_2nd_char
                g_tx_char_ev.set()
            elif ev == EV_TX_XON and g_is_xoff_sent:
                g_tx_state = "T1c" if g_is_xoff_recvd else "T0c"
            elif ev == EV_TX_XOFF and not g_is_xoff_sent:
                g_tx_state = "T1c" if g_is_xoff_recvd else "T0c"
            elif ev == EV_RX_XON:
                g_is_xoff_recvd = False
            elif ev == EV_RX_XOFF:
                g_is_xoff_recvd = True
        elif g_tx_state == "T2e":
            if ev == EV_TX_READY:
                g_tx_state = "T2f"
                g_tx_data = g_2nd_char
                g_tx_char_ev.set()
            elif ev == EV_RX_XOFF:
                g_tx_state = "T2d"
                g_is_xoff_recvd = True
            elif (ev == EV_TX_XON and g_is_xoff_sent) or \
                 (ev == EV_TX_XOFF and not g_is_xoff_sent):
                g_tx_state = "T4"
        elif g_tx_state == "T2f":
            if ev == EV_TX_READY:
                g_tx_state = "T2g"
                g_tx_data = ESC
                g_2nd_char = XOFF if g_is_xoff_sent else XON
                g_tx_char_ev.set()
            elif ev == EV_RX_XOFF:
                g_tx_state = "T2c"
                g_is_xoff_recvd = True
            elif (ev == EV_TX_XON and g_is_xoff_sent) or \
                 (ev == EV_TX_XOFF and not g_is_xoff_sent):
                g_tx_state = "T3"
        elif g_tx_state == "T2g":
            if ev == EV_TX_READY:
                g_tx_state = "T3"
                g_tx_data = g_2nd_char
                g_tx_char_ev.set()
            elif ev == EV_RX_XOFF:
                g_tx_state = "T2b"
                g_is_xoff_recvd = True
        elif g_tx_state == "T3":
            if ev == EV_TX_READY:
                if g_n_trmtd == g_max_n_trmtd:
                    if g_syn_on_close:
                        g_syn_on_close = False
                        g_tx_data = SYN
                        g_tx_char_ev.set()
                    else:
                        g_tx_state = "T0a"
                        g_write_end_ev.set()
                else:
                    ch = g_trmtd[g_n_trmtd]
                    g_n_trmtd += 1
                    if ch == SYN or ch == ESC:
                        logging.debug(f"Transmitting {ch}")
                        g_tx_state = "T4"
                        g_tx_data = ESC
                        g_2nd_char = ch
                        g_tx_char_ev.set()
                    else:
                        g_tx_data = ch
                        g_tx_char_ev.set()
            elif ev == EV_RX_XOFF:
                g_tx_state = "T1b"
                g_is_xoff_recvd = True
            elif ev == EV_TX_XOFF:
                g_tx_state = "T2f"
                g_is_xoff_sent = True
            elif ev == EV_TX_XON:
                g_tx_state = "T2f"
                g_is_xoff_sent = False
        elif g_tx_state == "T4":
            if ev == EV_TX_READY:
                g_tx_state = "T3"
                g_tx_data = g_2nd_char
                g_tx_char_ev.set()
            elif ev == EV_TX_XOFF:
                g_tx_state = "T2a"
                g_is_xoff_sent = True
            elif ev == EV_TX_XON:
                g_tx_state = "T2a"
                g_is_xoff_sent = False
            elif ev == EV_RX_XOFF:
                g_tx_state = "T1c"
                g_is_xoff_recvd = True
        else:
            logging.warning(f"Estado em TX desconhecido ({g_tx_state})")
            g_rx_state = "T0a"

    logging.debug("Saindo   da função tx_process_event()")

#
# Funções da API
#

def read_bytes(max_n_bytes, clear_on_entry=True):
    global g_recvd, g_n_recvd, g_max_n_recvd, g_read_end_ev
    global g_idle_buffer

    logging.debug("Entrando na função read_bytes()")

    g_recvd = bytearray()
    g_max_n_recvd = max_n_bytes
    if clear_on_entry:
        g_idle_buffer = bytearray()

    rx_process_event(EV_READ)

    g_read_end_ev.clear()
    g_read_end_ev.wait()

    logging.debug("Saindo   da função read_bytes()")
    return g_n_recvd, g_recvd

def write_bytes(trmtd, close_packet=True):
    global g_trmtd, g_n_trmtd, g_max_n_trmtd, g_write_end_ev, \
        g_syn_on_close

    logging.debug("Entrando na função write_bytes()")

    if len(trmtd) == 0:
        logging.debug("No bytes in buffer, transmitting nothing")
        return 0

    if g_xoff_active_ev.is_set():
        logging.debug("XOFF is active, transmitting nothing")
        return 0

    g_trmtd = trmtd
    g_n_trmtd, g_max_n_trmtd = 0, len(trmtd)
    g_syn_on_close = close_packet

    g_write_end_ev.clear()
    tx_process_event(EV_WRITE)
    g_write_end_ev.wait()

    logging.debug("Saindo  da função write_bytes()")
    return g_n_trmtd

def flow_off():
    logging.debug("Entrando na função flow_off()")

    tx_process_event(EV_TX_XOFF)

    logging.debug("Saindo  da função flow_off()")

def flow_on():
    logging.debug("Entrando na função flow_on()")

    tx_process_event(EV_TX_XON)

    logging.debug("Saindo  da função flow_on()")

#
# A parte principal usando as funções da API acima para implementar os
# testes descritos no texto
#

try:
    with serial.Serial(serial_name, 9600, timeout=1) as ser:
        logging.debug("Iniciando a thread de leitura")
        g_stop_all_ev.clear()
        rx_th = threading.Thread(target=read_daemon, args=(ser,))
        rx_th.start()
        logging.debug("Iniciada a thread de leitura")

        logging.debug("Iniciando a thread de escrita")
        g_tx_char_ev.clear()
        tx_th = threading.Thread(target=write_daemon, args=(ser,))
        tx_th.start()
        logging.debug("Iniciada a thread de escrita")

        # 1º teste - receber string com caractere de sincronismo no final
        logging.info("=== 1º teste")
        n, rcvd = read_bytes(100)
        logging.info(f"n = {n}, bytes recebidos = {rcvd}")

        # 2º teste - receber string enviada por partes com caractere de sincronismo no final
        logging.info("=== 2º teste")
        n, rcvd = read_bytes(100)
        logging.info(f"n = {n}, bytes recebidos = {rcvd}")

        input("Digite <Enter>")

        # 3º teste - enviar string repetidas vezes completando pelo
        # menos 360 caracteres sem caractere de sincronismo no final
        msg1 = b"0123456789"*4
        logging.info("=== 3º teste")
        for i in range(10):
            n = write_bytes(msg1, False)
            time.sleep(0.01)
            logging.info(f"#{i} - Enviados {n} bytes")

        input("Digite <Enter> (espere pelo outro lado)")

        # 4º teste - repetir o teste acima sem levar em conta o comando de Xoff
        logging.info("=== 4º teste")
        for i in range(10):
            n = ser.write(msg1)
            time.sleep(0.01)
            logging.info(f"#{i} - Enviados {n} bytes")

        input("Digite <Enter>")

        # 5º teste - receber string enviando comando de Xoff
        logging.info("=== 5º teste")
        for i in range(1):
            n, rcvd = read_bytes(10)
            logging.info(f"n = {n}, bytes recebidos = {rcvd}")
        flow_off()
        logging.info(f"Enviado Xoff")
        def wait_rx_on_xoff(event):
            logging.debug(f"Entrando na função wait_rx_on_xoff()")
            time.sleep(1)
            if not event.is_set():
                rx_process_event(EV_SYN)
                logging.info(f"Após XOFF, transmissão foi interrompida")
            else:
                logging.info(f"Após XOFF, transmissão NÃO foi interrompida")
            logging.debug(f"Saindo   da função wait_rx_on_xoff()")
        wait_ev = threading.Event()
        wait_ev.clear()
        wait_th = threading.Thread(target=wait_rx_on_xoff, args=(wait_ev,))
        wait_th.start()
        n, rcvd = read_bytes(30)
        wait_ev.set()
        wait_th.join()
        flow_on()
        logging.info(f"Retornado n = {n} e rcvd = {rcvd}")

        # 6º teste - Loop infinito com 2 tarefas
        msg2 = b"Desenvolvimento de sistemas embarcados"
        logging.info("=== 6º teste - parte a")
        while True:
            input("Digite Enter")
            logging.info("Writing message - parte a")
            n = write_bytes(msg2)
            logging.info(f"Escrito {n} bytes - parte a")

            input("Digite Enter")
            logging.info("Writing message - parte b")
            PCKT_SZ = 5
            upper_limit, n_total = ((len(msg2) - 1)//PCKT_SZ)*PCKT_SZ, 0
            for idx in range(0, upper_limit, PCKT_SZ):
                n = write_bytes(msg2[idx:idx+PCKT_SZ], close_packet=False)
                n_total += n
                logging.info(f"{n} bytes transmitidos na rodada #{idx//PCKT_SZ}")
            n = write_bytes(msg2[upper_limit:], close_packet=True)
            logging.info(f"Escrito {n_total} bytes - parte b")
finally:
    g_stop_all_ev.set()
    g_read_end_ev.set()
    g_write_end_ev.set()
    rx_th.join()
    tx_th.join()
