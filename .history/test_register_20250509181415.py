import socket

HOST = '127.0.0.1'
PORT = 5050

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))

    s.sendall(b'REGISTER\x00')
    nombre = "raul"
    #s.sendall(nombre.encode('utf-8') + b'\x00')
    print("Enviado, esperando respuesta...")

    respuesta = s.recv(4)
    print("Código de respuesta:", int.from_bytes(respuesta, byteorder='little'))