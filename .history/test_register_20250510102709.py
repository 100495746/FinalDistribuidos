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

# --- PRUEBA REGISTER ---
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.sendall(b'REGISTER\x00')
    s.sendall(b'raul\x00')

    print("Enviado REGISTER, esperando respuesta...")
    respuesta = s.recv(4)
    resultado = int.from_bytes(respuesta, byteorder='little', signed=True)
    print("REGISTER → Código de respuesta:", resultado)

    mensaje = readString(s)
    print("REGISTER → Mensaje recibido del servidor:", mensaje)


# --- PRUEBA CONNECT ---
import struct
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.sendall(b'CONNECT\x00')
    s.sendall(b'raul\x00')
    puerto = 4567
    s.sendall(struct.pack("i", puerto))

    print("Enviado CONNECT, esperando respuesta...")
    respuesta = s.recv(4)
    resultado = int.from_bytes(respuesta, byteorder='little', signed=True)
    print("CONNECT → Código de respuesta:", resultado)

    mensaje = readString(s)
    print("CONNECT → Mensaje recibido del servidor:", mensaje)


# --- PRUEBA PUBLISH ---
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.sendall(b'PUBLISH\x00')
    s.sendall(b'raul\x00')
    s.sendall(b'/tmp/files/f1\x00')
    s.sendall(b'foto de prueba\x00')

    print("Enviado PUBLISH, esperando respuesta...")
    respuesta = s.recv(4)
    resultado = int.from_bytes(respuesta, byteorder='little', signed=True)
    print("PUBLISH → Código de respuesta:", resultado)

    mensaje = readString(s)
    print("PUBLISH → Mensaje recibido del servidor:", mensaje)

# --- PRUEBA LIST_USERS ---
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.sendall(b'LIST_USERS\x00')

    print("Enviado LIST_USERS, esperando lista...")
    while True:
        linea = readString(s)
        if linea == "\n":
            break
        print("USUARIO:", linea.strip())


# --- PRUEBA LIST_CONTENT ---
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.sendall(b'LIST_CONTENT\x00')
    s.sendall(b'raul\x00')

    print("Enviado LIST_CONTENT, esperando lista...")
    while True:
        linea = readString(s)
        if linea == "\n":
            break
        print("FICHERO:", linea.strip())


# --- PRUEBA GET_FILE ---
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.sendall(b'GET_FILE\x00')
    s.sendall(b'raul\x00')
    s.sendall(b'/tmp/files/f1\x00')
    s.sendall(b'destino_local.txt\x00')

    print("Enviado GET_FILE, esperando confirmación...")
    respuesta = s.recv(4)
    resultado = int.from_bytes(respuesta, byteorder='little', signed=True)
    print("GET_FILE → Código de respuesta:", resultado)


# --- PRUEBA DISCONNECT ---
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.sendall(b'DISCONNECT\x00')
    s.sendall(b'raul\x00')

    print("Enviado DISCONNECT, esperando respuesta...")
    respuesta = s.recv(4)
    resultado = int.from_bytes(respuesta, byteorder='little', signed=True)
    print("DISCONNECT → Código de respuesta:", resultado)