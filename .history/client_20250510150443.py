from enum import Enum
import socket
import struct
import threading
import argparse
import os


class client :



    # ******************** TYPES *********************

    # *

    # * @brief Return codes for the protocol methods

    class RC(Enum) :
        # tipos de mensajes

        OK = 0

        ERROR = 1

        USER_ERROR = 2



    # ****************** ATTRIBUTES ******************

    _server = None

    _port = -1

    _current_user = None



    # ******************** METHODS *******************

    @staticmethod
    def readString(sock):
        #lee byte a byte
        a = ''
        while True:
            msg = sock.recv(1)
            if msg == b'\0':
                break
            a += msg.decode()
        return a

    @staticmethod
    def register(user):
        try:
            # Crea un socket TCP (IPv4) y lo gestiona automáticamente con el contexto 'with'.
            # Al salir del bloque, el socket se cierra automáticamente, incluso si ocurre una excepción.
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                # Envía todos los datos especificados al socket, asegurando que se transmita completamente.
                # En este caso, envía el comando 'REGISTER' seguido de un byte nulo como terminador.
                s.sendall(b'REGISTER\x00')
                s.sendall(user.encode() + b'\x00')
                fecha = client.get_datetime()
                s.sendall(fecha.encode() + b'\x00')

                response = s.recv(4)
                result = int.from_bytes(response, byteorder='little', signed=True)
                message = client.readString(s)

                print("REGISTER →", message)
                return client.RC.OK if result == 0 else client.RC.USER_ERROR
        except Exception as e:
            print("REGISTER Exception:", str(e))
            return client.RC.ERROR
        
    @staticmethod
    def connect(user):
        base_port = 4500
        port = base_port + (sum(ord(c) for c in user) % 1000)
        try:
            def file_server_thread():
                # Este hilo actúa como servidor de ficheros en el cliente
                
                listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                listener.bind(('', port))  # El puerto debe coincidir con el dado en CONNECT
                
                listener.listen(1)
                print(f"CLIENT FILE SERVER LISTENING on port {port}...")

                while True:
                    conn, addr = listener.accept()
                    print("Connection received from:", addr)

                    try:
                        # Leer comando y parámetros
                        cmd = client.readString(conn)
                        if cmd != "GET_FILE":
                            print("Invalid command received:", cmd)
                            conn.close()
                            continue

                        remote_path = client.readString(conn)
                        local_name = client.readString(conn)

                        print(f"GET_FILE request: path={remote_path}, dest={local_name}")

                        try:
                            #abrimos tcp
                            with open(remote_path, "rb") as f:
                                content = f.read()
                                conn.sendall(b'\x00')  # status OK
                                conn.sendall(f"{len(content)}\0".encode())
                                conn.sendall(content)
                        except FileNotFoundError:
                            conn.sendall(b'\x01')  # no existe
                        except:
                            conn.sendall(b'\x02')  # error

                    except Exception as e:
                        print("Error during GET_FILE handling:", str(e))

                    conn.close()

            # Iniciar hilo servidor de ficheros al conectar
            threading.Thread(target=file_server_thread, daemon=True).start()

            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(b'CONNECT\x00')
                s.sendall(user.encode() + b'\x00')
                s.sendall(struct.pack("i", port))

                
                fecha = client.get_datetime()
                s.sendall(fecha.encode() + b'\x00')

                response = s.recv(4)
                result = int.from_bytes(response, byteorder='little', signed=True)
                message = client.readString(s)

                print("CONNECT →", message)
                client._current_user = user

                return client.RC.OK if result == 0 else client.RC.USER_ERROR
        except Exception as e:
            print("CONNECT Exception:", str(e))
            return client.RC.ERROR

   

    @staticmethod
    def unregister(user):
        try:
            if client._current_user is None:
                print("Error: no user connected.")
                return client.RC.USER_ERROR
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(b'UNREGISTER\x00')
                s.sendall(user.encode() + b'\x00')
                fecha = client.get_datetime()
                s.sendall(fecha.encode() + b'\x00')

                response = s.recv(4)
                result = int.from_bytes(response, byteorder='little', signed=True)
                print("UNREGISTER → Resultado:", result)
                return client.RC.OK if result == 0 else client.RC.USER_ERROR
        except Exception as e:
            print("UNREGISTER Exception:", str(e))
            return client.RC.ERROR






    

    @staticmethod
    def disconnect(user):
        try:
            if client._current_user is None:
                print("Error: no user connected.")
                return client.RC.USER_ERROR
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(b'DISCONNECT\x00')
                s.sendall(user.encode() + b'\x00')
                fecha = client.get_datetime()
                s.sendall(fecha.encode() + b'\x00')

                response = s.recv(4)
                result = int.from_bytes(response, byteorder='little', signed=True)
                print("DISCONNECT → Resultado:", result)
                return client.RC.OK if result == 0 else client.RC.USER_ERROR
        except Exception as e:
            print("DISCONNECT Exception:", str(e))
            return client.RC.ERROR



    @staticmethod
    def publish(fileName, description):
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(b'PUBLISH\x00')
                s.sendall(b'raul\x00')  # sustituir por el usuario si gestionas sesión
                abs_path = os.path.abspath(fileName)
                s.sendall(abs_path.encode() + b'\x00')
                s.sendall(description.encode() + b'\x00')
                fecha = client.get_datetime()
                s.sendall(fecha.encode() + b'\x00')

                response = s.recv(4)
                result = int.from_bytes(response, byteorder='little', signed=True)
                message = client.readString(s)

                print("PUBLISH →", message)
                # Devuelve un código de retorno según el resultado de la operación:
                # client.RC.OK si fue exitosa, client.RC.USER_ERROR si hubo un error de usuario.

                return client.RC.OK if result == 0 else client.RC.USER_ERROR
        except Exception as e:
            print("PUBLISH Exception:", str(e))
            return client.RC.ERROR




    @staticmethod
    def listusers():
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(b'LIST_USERS\x00')
                fecha = client.get_datetime()
                s.sendall(fecha.encode() + b'\x00')

                print("LIST_USERS →")
                while True:
                    line = client.readString(s)
                    if line == "\n":
                        break
                    print("  " + line.strip())
                return client.RC.OK
        except Exception as e:
            print("LIST_USERS Exception:", str(e))
            return client.RC.ERROR


    @staticmethod
    def delete(fileName):
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(b'DELETE\x00')
                if client._current_user is None:
                    print("Error: no user connected.")
                    return client.RC.USER_ERROR
                s.sendall(client._current_user.encode() + b'\x00')
                abs_path = os.path.abspath(fileName)
                s.sendall(abs_path.encode() + b'\x00')
                fecha = client.get_datetime()
                s.sendall(fecha.encode() + b'\x00')

                response = s.recv(4)
                result = int.from_bytes(response, byteorder='little', signed=True)
                print("DELETE → Resultado:", result)
                return client.RC.OK if result == 0 else client.RC.USER_ERROR
        except Exception as e:
            print("DELETE Exception:", str(e))
            return client.RC.ERROR


    @staticmethod
    def listcontent(user):
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(b'LIST_CONTENT\x00')
                s.sendall(user.encode() + b'\x00')
                fecha = client.get_datetime()
                s.sendall(fecha.encode() + b'\x00')              

                print("LIST_CONTENT →")
                while True:
                    line = client.readString(s)
                    if line == "\n":
                        break
                    print("  " + line.strip())
                return client.RC.OK
        except Exception as e:
            print("LIST_CONTENT Exception:", str(e))
            return client.RC.ERROR


    @staticmethod
    def getfile(user, remote_FileName, local_FileName):
        import os  # Asegúrate de que esté importado arriba si no lo está
        base_port = 4500
        port = base_port + (sum(ord(c) for c in user) % 1000)
        try:
            # Convertir a rutas absolutas
            remote_FileName = os.path.abspath(remote_FileName)
            local_FileName = os.path.abspath(local_FileName)

            # 1. Solicitar permiso al servidor central
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(b'GET_FILE\x00')
                s.sendall(user.encode() + b'\x00')
                s.sendall(remote_FileName.encode() + b'\x00')
                s.sendall(local_FileName.encode() + b'\x00')
                fecha = client.get_datetime()
                s.sendall(fecha.encode() + b'\x00')

                response = s.recv(4)
                result = int.from_bytes(response, byteorder='little', signed=True)
                if result != 0:
                    print("GET_FILE → Server denied or user unavailable.")
                    return client.RC.USER_ERROR

            # 2. Conectarse al cliente remoto para pedir el fichero
            ip = '127.0.0.1'  # Hardcoded por ahora

            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as peer:
                peer.connect((ip, port))
                peer.sendall(b'GET_FILE\x00')
                peer.sendall(remote_FileName.encode() + b'\x00')
                peer.sendall(local_FileName.encode() + b'\x00')
                

                status = peer.recv(1)
                if status == b'\x00':
                    size = client.readString(peer)
                    content = peer.recv(int(size))
                    with open(local_FileName, "wb") as f:
                        f.write(content)
                    print("GET_FILE → Fichero recibido correctamente.")
                    return client.RC.OK
                elif status == b'\x01':
                    print("GET_FILE → El fichero no existe.")
                    return client.RC.USER_ERROR
                else:
                    print("GET_FILE → Error en el cliente remoto.")
                    return client.RC.ERROR

        except Exception as e:
            print("GET_FILE Exception:", str(e))
            return client.RC.ERROR


    @staticmethod
    def get_datetime():
        try:
            import requests
            response = requests.get("http://127.0.0.1:5052/datetime")
            return response.text
        except:
            return "FECHA NO DISPONIBLE"

    # *

    # **

    # * @brief Command interpreter for the client. It calls the protocol functions.

    @staticmethod

    def shell():



        while (True) :

            try :

                command = input("c> ")

                line = command.split(" ")

                if (len(line) > 0):



                    line[0] = line[0].upper()



                    if (line[0]=="REGISTER") :

                        if (len(line) == 2) :

                            client.register(line[1])

                        else :

                            print("Syntax error. Usage: REGISTER <userName>")



                    elif(line[0]=="UNREGISTER") :

                        if (len(line) == 2) :

                            client.unregister(line[1])
                        if client._current_user is None:
                            print("Error: no user connected.")
                        elif len(line) == 1:
                            client.unregister(client._current_user)

                        else :

                            print("Syntax error. Usage: UNREGISTER <userName>")



                    elif(line[0]=="CONNECT") :

                        if (len(line) == 2) :

                            client.connect(line[1])

                        else :

                            print("Syntax error. Usage: CONNECT <userName>")

                    

                    elif(line[0]=="PUBLISH") :

                        if (len(line) >= 3) :

                            #  Remove first two words

                            description = ' '.join(line[2:])

                            client.publish(line[1], description)

                        else :

                            print("Syntax error. Usage: PUBLISH <fileName> <description>")



                    elif(line[0]=="DELETE") :

                        if (len(line) == 2) :

                            client.delete(line[1])

                        else :

                            print("Syntax error. Usage: DELETE <fileName>")



                    elif(line[0]=="LIST_USERS") :

                        if (len(line) == 1) :

                            client.listusers()

                        else :

                            print("Syntax error. Use: LIST_USERS")



                    elif(line[0] == "LIST_CONTENT") :
                        if len(line) == 2:
                            client.listcontent(line[1])
                        elif len(line) == 1 and client._current_user is not None:
                            client.listcontent(client._current_user)
                        else:
                            print("Syntax error. Usage: LIST_CONTENT <userName>")



                    elif(line[0]=="DISCONNECT") :
                        if client._current_user is None:
                            print("Error: no user connected.")
                        elif len(line) == 1:
                            client.disconnect(client._current_user)
                        else:
                            print("Syntax error. Use: DISCONNECT")



                    elif(line[0]=="GET_FILE") :

                        if (len(line) == 4) :

                            client.getfile(line[1], line[2], line[3])

                        else :

                            print("Syntax error. Usage: GET_FILE <userName> <remote_fileName> <local_fileName>")



                    elif(line[0]=="QUIT") :

                        if (len(line) == 1) :

                            break

                        else :

                            print("Syntax error. Use: QUIT")

                    else :

                        print("Error: command " + line[0] + " not valid.")

            except Exception as e:

                print("Exception: " + str(e))



    # *

    # * @brief Prints program usage

    @staticmethod

    def usage() :

        print("Usage: python3 client.py -s <server> -p <port>")





    # *

    # * @brief Parses program execution arguments

    @staticmethod

    def  parseArguments(argv) :

        parser = argparse.ArgumentParser()

        parser.add_argument('-s', type=str, required=True, help='Server IP')

        parser.add_argument('-p', type=int, required=True, help='Server Port')

        args = parser.parse_args()



        if (args.s is None):

            parser.error("Usage: python3 client.py -s <server> -p <port>")

            return False



        if ((args.p < 1024) or (args.p > 65535)):

            parser.error("Error: Port must be in the range 1024 <= port <= 65535");

            return False;

        

        client._server = args.s

        client._port = args.p



        return True





    # ******************** MAIN *********************

    @staticmethod

    def main(argv) :

        if (not client.parseArguments(argv)) :

            client.usage()

            return



        #  Write code here

        client.shell()

        print("+++ FINISHED +++")

    



if __name__=="__main__":

    client.main([])