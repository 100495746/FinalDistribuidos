import socket
import time

HOST = '127.0.0.1'
PORT = 5051

def readString(sock):
    a = ''
    while True:
        msg = sock.recv(1)
        if msg == b'\0':
            break
        a += msg.decode()
    return a

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))

    # Enviar comando y nombre juntos en un solo sendall, ambos terminados en \0
    s.sendall(b'REGISTER\x00')
    s.sendall(b'raul\x00')

    print("Enviado, esperando respuesta...")
    respuesta = s.recv(4)
    print("Código de respuesta:", int.from_bytes(respuesta, byteorder='little'))

    # Leer mensaje textual adicional del servidor
    mensaje = readString(s)
    print("Mensaje recibido del servidor:", mensaje)