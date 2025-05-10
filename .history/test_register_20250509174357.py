import socket

HOST = '127.0.0.1'
PORT = 5000

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))

    # Enviar comando "REGISTER" con \0
    s.sendall(b'REGISTER\x00')

    # Enviar nombre de usuario "raul" con \0
    s.sendall(b'raul\x00')

    # Leer respuesta del servidor (entero)
    respuesta = s.recv(4)
    print("Código de respuesta:", int.from_bytes(respuesta, byteorder='little'))