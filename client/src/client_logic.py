from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *
from socket import *
import threading
import re

class client_connection(QObject):
    host = 'localhost'
    port = 21
    bufsize = 1024
    addr = (host, port)

    data_ip = ''
    data_port = 0
    pasv_or_port = False  # true for port, false for pasv
    retr_or_stor = False  # true for stor, false for retr
    is_transfering = False
    connection_prepared = False
    paused = False
    rw_offset = 0   # used for transfer file
    transfering_file = ''
    server_working_path = ''

    listen_socket = socket()
    data_socket = socket()
    client_socket = socket(AF_INET, SOCK_STREAM)  # used for command

    err_sig = pyqtSignal(str)
    response_sig = pyqtSignal(str)
    list_sig = pyqtSignal(str,list)
    transfre_size_sig = pyqtSignal(int)
    transfer_finished = pyqtSignal()
    def __init__(self):
        super().__init__()

    def establish_connection(self, ip, port, username, password):
        self.client_socket = socket(AF_INET, SOCK_STREAM)
        self.listen_socket = socket()
        self.data_socket = socket()
        addr = (ip, port)
        self.client_socket.connect(addr)
        response = self.client_socket.recv(self.bufsize).decode('utf8')
        if not response.startswith('220'):
            self.err_sig.emit(response)
            return
        request = 'USER '+username+'\r\n'
        self.client_socket.send(request.encode('utf8'))
        response = self.client_socket.recv(self.bufsize).decode('utf8')
        if not response.startswith('331'):
            self.err_sig.emit(response)
            return
        request = 'PASS '+password+'\r\n'
        self.client_socket.send(request.encode('utf8'))
        response = self.client_socket.recv(self.bufsize).decode('utf8')
        if not response.startswith('230'):
            self.err_sig.emit(response)
            return
        request = 'TYPE I\r\n'
        self.client_socket.send(request.encode('utf8'))
        response = self.client_socket.recv(self.bufsize).decode('utf8')
        if not response.startswith('200'):
            self.err_sig.emit(response)
            return

        self.get_dir_info()

    def shutdown_connection(self):
        if self.is_transfering:
            self.err_sig.emit('data transfering')
            return
        request = 'QUIT\r\n'
        self.client_socket.send(request.encode('utf8'))
        response = self.client_socket.recv(self.bufsize).decode('utf8')
        if not response.startswith('221'):
            self.err_sig.emit(response)
        self.client_socket.close()
        self.listen_socket.close()


    def get_dir_info(self):
        request = 'PWD\r\n'
        self.client_socket.send(request.encode('utf8'))
        response = self.client_socket.recv(self.bufsize).decode('utf8')
        if not response.startswith('257'):
            self.err_sig.emit(response)
        self.server_working_path = response.split()[1].replace('\r\n', '')

        if self.pasv_or_port:
            self.port_connection()
        else:
            self.pasv_connection()

        self.get_dir_list()

    def syst_request(self):
        if self.is_transfering:
            self.err_sig.emit('data transfering')
            return
        request = 'SYST\r\n'
        self.client_socket.send(request.encode('utf8'))
        response = self.client_socket.recv(self.bufsize).decode('utf8')
        if not response.startswith('215'):
            self.err_sig.emit(response)
            return
        response = response.replace('\r\n', '').split()
        msg=' '.join(response[1:])
        self.response_sig.emit(msg)


    def pasv_connection(self):
        request = 'PASV'+'\r\n'
        response = self.client_socket.send(request.encode('utf8'))
        response = self.client_socket.recv(self.bufsize).decode('utf8')
        ip_port_pattern = r'(\d{1,3}),(\d{1,3}),(\d{1,3}),(\d{1,3}),(\d{1,3}),(\d{1,3})'
        if response.startswith('227'):
            ip_port_match = re.search(ip_port_pattern, response)
            self.data_ip = ip_port_match.group(1) + '.'
            self.data_ip += ip_port_match.group(2) + '.'
            self.data_ip += ip_port_match.group(3) + '.'
            self.data_ip += ip_port_match.group(4)
            self.data_port = int(ip_port_match.group(5))*256+int(ip_port_match.group(6))
            data_addr = (self.data_ip, self.data_port)
            self.data_socket = socket(AF_INET, SOCK_STREAM)
            self.data_socket.connect(data_addr)
            self.pasv_or_port = False
            self.connection_prepared = True
        else:
            self.err_sig.emit('PASV error')

    def port_connection(self):
        ip_parts = self.data_ip.split('.')
        req_ip = ','.join(ip_parts)
        req_port = str(int(self.data_port/256))+','+str(int(self.data_port%256))
        request = 'PORT '+req_ip+','+req_port+'\r\n'
        data_ip_port = (self.data_ip, self.data_port)
        self.listen_socket.bind(data_ip_port)
        self.listen_socket.listen(1)

        self.client_socket.send(request.encode('utf8'))
        response = self.client_socket.recv(self.bufsize).decode('utf8').replace('\r\n', '')
        if response.startswith('200'):
            self.connection_prepared = True
        else:
            self.err_sig.emit(response)

    def set_data_connection_default(self):
        self.is_transfering = False
        self.connection_prepared = False
        self.data_socket.shutdown(2)
        self.data_socket.close()

    def change_dir(self, target_dir):
        if self.is_transfering:
            self.err_sig.emit('data transfering')
            return
        request = 'CWD '+target_dir+'\r\n'
        self.client_socket.send(request.encode('utf8'))
        response = self.client_socket.recv(self.bufsize).decode('utf8').replace('\r\n', '')
        if not response.startswith('250'):
            self.err_sig.emit(response)
            return
        self.get_dir_info()

    def get_dir_list(self):
        # get list of dir
        if not self.connection_prepared:
            self.err_sig.emit('no valid data connection')
        else:
            self.is_transfering = True
            request = 'LIST\r\n'
            list_thread = threading.Thread(target=self.transfer_list_data, args=(request,))
            list_thread.start()

    def transfer_list_data(self, req):
        if self.pasv_or_port:
            self.client_socket.send(req.encode('utf8'))
            self.listen_socket.settimeout(10)
            self.data_socket, _ = self.listen_socket.accept()
            response = self.client_socket.recv(self.bufsize).decode('utf8')
        else:
            self.client_socket.send(req.encode('utf8'))
            response = self.client_socket.recv(self.bufsize).decode('utf8')

        if not response.startswith('150'):
            self.set_data_connection_default()
            self.err_sig.emit(response.replace('\r\n', ''))
            return

        dir_list = []
        response = self.data_socket.recv(self.bufsize, MSG_PEEK)
        while len(response)>0:
            single_msg_size=response.find(b'\r\n')+2
            file_msg=self.data_socket.recv(single_msg_size).decode('utf8')

            file_msg.replace('\r\n', '')
            # print(file_msg)
            dir_list.append(file_msg)

            # dir_list+=response.split('\n')
            response = self.data_socket.recv(self.bufsize, MSG_PEEK)

        self.is_transfering = False
        self.connection_prepared = False
        self.data_socket.shutdown(2)
        self.data_socket.close()
        response = self.client_socket.recv(self.bufsize).decode('utf8')



        self.list_sig.emit(self.server_working_path, dir_list)

        return

    def mode_change(self, mode, data_ip, data_port):
        if mode == 'PORT':
            self.pasv_or_port = True
            self.data_ip = data_ip
            self.data_port = data_port
        elif mode == 'PASV':
            self.pasv_or_port = False


    def upload_file(self, file_name):
        if self.is_transfering:
            self.err_sig.emit('data transfering')
            return
        if self.pasv_or_port:
            self.port_connection()
        else:
            self.pasv_connection()

        if not self.connection_prepared:
            self.err_sig.emit('no valid data connection')
            return
        self.is_transfering = True
        request = 'STOR '+ file_name.split('/')[-1]+'\r\n'
        self.transfering_file = file_name
        self.rw_offset = 0
        self.retr_or_stor = True
        file_transfer_thread = threading.Thread(target=self.stor_data_transfer, args=(request, ))
        file_transfer_thread.start()


    def stor_data_transfer(self, req):
        file_name = req.split()[-1]
        tar_file = open(self.transfering_file, mode='rb')
        if tar_file is None:
            self.err_sig.emit('no such local file')
            self.set_data_connection_default()
            return
        if self.pasv_or_port:
            self.client_socket.send(req.encode('utf8'))
            #self.listen_socket.settimeout(10)
            self.data_socket, _ = self.listen_socket.accept()
            response = self.client_socket.recv(self.bufsize).decode('utf8')
        else:
            self.client_socket.send(req.encode('utf8'))
            response = self.client_socket.recv(self.bufsize).decode('utf8')
        if not response.startswith('150'):
            self.set_data_connection_default()
            self.err_sig.emit(response.replace('\r\n', ''))
            self.set_data_connection_default()
            return

        while True:
            if self.paused:
                break
            stor_data = tar_file.read(self.bufsize)
            if len(stor_data)<=0:
                self.rw_offset = 0
                self.transfering_file = ''
                break
            self.rw_offset+=len(stor_data)
            self.data_socket.send(stor_data)
            self.transfre_size_sig.emit(self.rw_offset)

        tar_file.close()
        self.set_data_connection_default()
        response = self.client_socket.recv(self.bufsize).decode('utf8')
        if self.transfering_file=='':
            self.transfer_finished.emit()
        return

    def download_file(self, file_name):
        if self.is_transfering:
            self.err_sig.emit('data transfering')
            return
        if self.pasv_or_port:
            self.port_connection()
        else:
            self.pasv_connection()

        if not self.connection_prepared:
            self.err_sig.emit('no valid data connection')
            return
        self.is_transfering = True
        request = 'RETR ' + file_name + '\r\n'
        self.transfering_file = file_name
        self.rw_offset = 0
        self.retr_or_stor = False
        file_transfer_thread = threading.Thread(target=self.retr_data_transfer, args=(request,))
        file_transfer_thread.start()

    def retr_data_transfer(self, req):
        file_name = req.split()[-1]
        if '/' in file_name:
            file_name = file_name.split('/')[-1]

        tar_file = open(file_name, mode='wb')
        if tar_file is None:
            self.err_sig.emit('no such local file')
            self.set_data_connection_default()
            return
        if self.pasv_or_port:
            self.client_socket.send(req.encode('utf8'))
            self.listen_socket.settimeout(10)
            self.data_socket, _ = self.listen_socket.accept()
            response = self.client_socket.recv(self.bufsize).decode('utf8')
        else:
            self.client_socket.send(req.encode('utf8'))
            response = self.client_socket.recv(self.bufsize).decode('utf8')

        if not response.startswith('150'):
            self.set_data_connection_default()
            self.err_sig.emit(response.replace('\r\n', ''))
            self.set_data_connection_default()
            return

        while True:
            if self.paused:
                break
            response = self.data_socket.recv(self.bufsize)
            if len(response)<=0:
                self.rw_offset = 0
                self.transfering_file = ''
                break
            self.rw_offset+=len(response)
            tar_file.write(response)
            self.transfre_size_sig.emit(self.rw_offset)

        print('out')
        tar_file.close()
        self.set_data_connection_default()
        response = self.client_socket.recv(self.bufsize)
        if self.transfering_file=='':
            self.transfer_finished.emit()
        return

    def pause_transfer(self):
        self.paused = True

    def continue_transfer(self):
        if self.is_transfering:
            self.err_sig.emit('data transfering')
            return
        if self.pasv_or_port:
            self.port_connection()
        else:
            self.pasv_connection()

        if not self.connection_prepared:
            self.err_sig.emit('no valid data connection')
            return
        self.is_transfering = True
        self.paused = False
        request = 'REST '+str(self.rw_offset)+'\r\n'
        file_transfer_thread = threading.Thread(target=self.rest_data_transfer, args=(request,))
        file_transfer_thread.start()


    def rest_data_transfer(self, req):

        file_name = self.transfering_file
        self.client_socket.send(req.encode('utf8'))
        response = self.client_socket.recv(self.bufsize).decode('utf8')
        if not response.startswith('350'):

            self.err_sig.emit(response.replace('\r\n', ''))
            return
        if self.retr_or_stor:
            request = 'STOR '+file_name+'\r\n'
        else:
            request = 'RETR ' + file_name + '\r\n'
        if self.pasv_or_port:
            self.client_socket.send(request.encode('utf8'))
            self.listen_socket.settimeout(10)
            self.data_socket, _ = self.listen_socket.accept()
            response = self.client_socket.recv(self.bufsize).decode('utf8')
        else:
            self.client_socket.send(request.encode('utf8'))
            response = self.client_socket.recv(self.bufsize).decode('utf8')

        if not response.startswith('150'):

            self.set_data_connection_default()
            self.err_sig.emit(response.replace('\r\n', ''))
            return

        if self.retr_or_stor:   #stor
            tar_file = open(file_name, mode='rb')

        else:
            tar_file = open(file_name, mode='ab')

        tar_file.seek(self.rw_offset)
        while True:
            if self.paused:
                break
            if self.retr_or_stor:
                stor_data = tar_file.read(self.bufsize)
                if len(stor_data)<=0:
                    self.rw_offset = 0
                    self.transfering_file = ''
                    break
                self.rw_offset+=len(stor_data)
                self.data_socket.send(stor_data)
                self.transfre_size_sig.emit(self.rw_offset)
            else:
                response = self.data_socket.recv(self.bufsize)
                if len(response) <= 0:
                    self.rw_offset = 0
                    self.transfering_file = ''
                    break
                self.rw_offset += len(response)
                tar_file.write(response)
                self.transfre_size_sig.emit(self.rw_offset)

        tar_file.close()
        self.set_data_connection_default()
        response = self.client_socket.recv(self.bufsize).decode('utf8')
        if self.transfering_file=='':
            self.transfer_finished.emit()
        return


    def move_file(self, source, target):
        if self.is_transfering:
            self.err_sig.emit('data transfering')
            return
        request = 'RNFR '+source+'\r\n'
        self.client_socket.send(request.encode('utf8'))
        response = self.client_socket.recv(self.bufsize).decode('utf8')
        if not response.startswith('350'):
            self.err_sig.emit(response)
            return
        request = 'RNTO '+target+'\r\n'
        self.client_socket.send(request.encode('utf8'))
        response = self.client_socket.recv(self.bufsize).decode('utf8')
        if not response.startswith('250'):
            self.err_sig.emit(response)
            return
        self.get_dir_info()

    def make_dir(self, dir_name):
        if self.is_transfering:
            self.err_sig.emit('data transfering')
            return
        request = 'MKD '+dir_name+'\r\n'
        self.client_socket.send(request.encode('utf8'))
        response = self.client_socket.recv(self.bufsize).decode('utf8')
        if not response.startswith('250'):
            self.err_sig.emit(response)
            return
        self.get_dir_info()

    def rm_file(self, file_name):
        if self.is_transfering:
            self.err_sig.emit('data transfering')
            return
        request = 'RMD '+file_name+'\r\n'
        self.client_socket.send(request.encode('utf8'))
        response = self.client_socket.recv(self.bufsize).decode('utf8')
        if not response.startswith('250'):
            self.err_sig.emit(response)
            return
        self.get_dir_info()
