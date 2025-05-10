from enum import Enum
import socket
import struct
import threading
import argparse



class client :



    # ******************** TYPES *********************

    # *

    # * @brief Return codes for the protocol methods

    class RC(Enum) :

        OK = 0

        ERROR = 1

        USER_ERROR = 2



    # ****************** ATTRIBUTES ******************

    _server = None

    _port = -1



    # ******************** METHODS *******************

    @staticmethod
    def readString(sock):
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
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(b'REGISTER\x00')
                s.sendall(user.encode() + b'\x00')

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
        try:
            def file_server_thread():
                listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                listener.bind(('', 4567))
                listener.listen(1)
                print("CLIENT FILE SERVER LISTENING on port 4567...")
                while True:
                    conn, addr = listener.accept()
                    print("Connection received from:", addr)
                    conn.close()

            threading.Thread(target=file_server_thread, daemon=True).start()

            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(b'CONNECT\x00')
                s.sendall(user.encode() + b'\x00')
                s.sendall(struct.pack("i", 4567))

                response = s.recv(4)
                result = int.from_bytes(response, byteorder='little', signed=True)
                message = client.readString(s)

                print("CONNECT →", message)
                return client.RC.OK if result == 0 else client.RC.USER_ERROR
        except Exception as e:
            print("CONNECT Exception:", str(e))
            return client.RC.ERROR

   

    @staticmethod

    def  unregister(user) :

        #  Write your code here

        return client.RC.ERROR





    





    

@staticmethod
def disconnect(user):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((client._server, client._port))
            s.sendall(b'DISCONNECT\x00')
            s.sendall(user.encode() + b'\x00')

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
                s.sendall(fileName.encode() + b'\x00')
                s.sendall(description.encode() + b'\x00')

                response = s.recv(4)
                result = int.from_bytes(response, byteorder='little', signed=True)
                message = client.readString(s)

                print("PUBLISH →", message)
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

    def  delete(fileName) :

        #  Write your code here

        return client.RC.ERROR


    @staticmethod
    def listcontent(user):
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((client._server, client._port))
                s.sendall(b'LIST_CONTENT\x00')
                s.sendall(user.encode() + b'\x00')

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

    def  getfile(user,  remote_FileName,  local_FileName) :

        #  Write your code here

        return client.RC.ERROR



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



                    elif(line[0]=="LIST_CONTENT") :

                        if (len(line) == 2) :

                            client.listcontent(line[1])

                        else :

                            print("Syntax error. Usage: LIST_CONTENT <userName>")



                    elif(line[0]=="DISCONNECT") :

                        if (len(line) == 2) :

                            client.disconnect(line[1])

                        else :

                            print("Syntax error. Usage: DISCONNECT <userName>")



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