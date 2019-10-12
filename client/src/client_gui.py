# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'client_gui.ui'
#
# Created by: PyQt5 UI code generator 5.11.3
#
# WARNING! All changes made in this file will be lost!

from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *
from client_logic import client_connection
from util import my_table_widget
import re
import os

class Ui_client_gui(QWidget):
    login_signal = pyqtSignal(str, int, str, str)
    logout_sig = pyqtSignal()
    mode_signal = pyqtSignal(str, str, int)
    move_sig = pyqtSignal(str, str) # used for rnfr and rnto
    make_dir_sig = pyqtSignal(str)
    rm_file_sig = pyqtSignal(str)
    stor_file_sig = pyqtSignal(str)
    retr_file_sig = pyqtSignal(str)
    pause_sig = pyqtSignal()
    continue_sig = pyqtSignal()
    refresh_sig = pyqtSignal()

    server_cwd = ''
    server_root = ''
    rnfr_source = ''
    rnto_target = ''
    err_msg=''
    server_mode = False  # true for standard server, false for customized server
    transfer_list = []
    def setupUi(self, client_gui):
        client_gui.setObjectName("client_gui")
        client_gui.resize(1280, 960)
        self.centralWidget = QtWidgets.QWidget(client_gui)
        self.centralWidget.setObjectName("centralWidget")
        self.systButton = QtWidgets.QPushButton(self.centralWidget)
        self.systButton.setGeometry(QtCore.QRect(1010, 180, 150, 30))
        self.systButton.setObjectName("systButton")
        self.horizontalLayoutWidget = QtWidgets.QWidget(self.centralWidget)
        self.horizontalLayoutWidget.setGeometry(QtCore.QRect(150, 30, 1021, 71))
        self.horizontalLayoutWidget.setObjectName("horizontalLayoutWidget")
        self.loginLayout = QtWidgets.QHBoxLayout(self.horizontalLayoutWidget)
        self.loginLayout.setContentsMargins(11, 11, 11, 11)
        self.loginLayout.setSpacing(6)
        self.loginLayout.setObjectName("loginLayout")
        self.command_ip_label = QtWidgets.QLabel(self.horizontalLayoutWidget)
        font = QtGui.QFont()
        font.setPointSize(20)
        self.command_ip_label.setFont(font)
        self.command_ip_label.setTextInteractionFlags(QtCore.Qt.LinksAccessibleByMouse)
        self.command_ip_label.setObjectName("command_ip_label")
        self.loginLayout.addWidget(self.command_ip_label)
        self.command_ipEdit = QtWidgets.QLineEdit(self.horizontalLayoutWidget)
        self.command_ipEdit.setObjectName("command_ipEdit")
        self.loginLayout.addWidget(self.command_ipEdit)
        self.portlabel = QtWidgets.QLabel(self.horizontalLayoutWidget)
        font = QtGui.QFont()
        font.setPointSize(20)
        self.portlabel.setFont(font)
        self.portlabel.setObjectName("portlabel")
        self.loginLayout.addWidget(self.portlabel)
        self.command_portEdit = QtWidgets.QLineEdit(self.horizontalLayoutWidget)
        self.command_portEdit.setObjectName("command_portEdit")
        self.loginLayout.addWidget(self.command_portEdit)
        self.usernamelabel = QtWidgets.QLabel(self.horizontalLayoutWidget)
        font = QtGui.QFont()
        font.setPointSize(20)
        self.usernamelabel.setFont(font)
        self.usernamelabel.setObjectName("usernamelabel")
        self.loginLayout.addWidget(self.usernamelabel)
        self.usernameEdit = QtWidgets.QLineEdit(self.horizontalLayoutWidget)
        self.usernameEdit.setObjectName("usernameEdit")
        self.loginLayout.addWidget(self.usernameEdit)
        self.passwordlabel = QtWidgets.QLabel(self.horizontalLayoutWidget)
        font = QtGui.QFont()
        font.setPointSize(20)
        self.passwordlabel.setFont(font)
        self.passwordlabel.setObjectName("passwordlabel")
        self.loginLayout.addWidget(self.passwordlabel)
        self.passwordEdit = QtWidgets.QLineEdit(self.horizontalLayoutWidget)
        self.passwordEdit.setObjectName("passwordEdit")
        self.loginLayout.addWidget(self.passwordEdit)
        self.choose_serverButton = QtWidgets.QPushButton(self.horizontalLayoutWidget)
        self.choose_serverButton.setObjectName("choose_serverButton")
        self.loginLayout.addWidget(self.choose_serverButton)
        self.loginButton = QtWidgets.QPushButton(self.horizontalLayoutWidget)
        self.loginButton.setObjectName("loginButton")
        self.loginLayout.addWidget(self.loginButton)
        self.horizontalLayoutWidget_2 = QtWidgets.QWidget(self.centralWidget)
        self.horizontalLayoutWidget_2.setGeometry(QtCore.QRect(150, 380, 881, 91))
        self.horizontalLayoutWidget_2.setObjectName("horizontalLayoutWidget_2")
        self.connectLayout = QtWidgets.QHBoxLayout(self.horizontalLayoutWidget_2)
        self.connectLayout.setContentsMargins(11, 11, 11, 11)
        self.connectLayout.setSpacing(6)
        self.connectLayout.setObjectName("connectLayout")
        self.data_iplabel = QtWidgets.QLabel(self.horizontalLayoutWidget_2)
        font = QtGui.QFont()
        font.setPointSize(20)
        self.data_iplabel.setFont(font)
        self.data_iplabel.setObjectName("data_iplabel")
        self.connectLayout.addWidget(self.data_iplabel)
        self.data_ipEdit = QtWidgets.QLineEdit(self.horizontalLayoutWidget_2)
        self.data_ipEdit.setText("")
        self.data_ipEdit.setObjectName("data_ipEdit")
        self.connectLayout.addWidget(self.data_ipEdit)
        self.data_portlabel = QtWidgets.QLabel(self.horizontalLayoutWidget_2)
        font = QtGui.QFont()
        font.setPointSize(20)
        self.data_portlabel.setFont(font)
        self.data_portlabel.setObjectName("data_portlabel")
        self.connectLayout.addWidget(self.data_portlabel)
        self.data_portEdit = QtWidgets.QLineEdit(self.horizontalLayoutWidget_2)
        self.data_portEdit.setText("")
        self.data_portEdit.setObjectName("data_portEdit")
        self.connectLayout.addWidget(self.data_portEdit)
        self.switchButton = QtWidgets.QPushButton(self.horizontalLayoutWidget_2)
        self.switchButton.setObjectName("switchButton")
        self.connectLayout.addWidget(self.switchButton)
        self.modelabel = QtWidgets.QLabel(self.horizontalLayoutWidget_2)
        font = QtGui.QFont()
        font.setPointSize(20)
        self.modelabel.setFont(font)
        self.modelabel.setObjectName("modelabel")
        self.connectLayout.addWidget(self.modelabel)
        self.confirmButton = QtWidgets.QPushButton(self.horizontalLayoutWidget_2)
        self.confirmButton.setObjectName("confirmButton")
        self.connectLayout.addWidget(self.confirmButton)
        self.horizontalLayoutWidget_3 = QtWidgets.QWidget(self.centralWidget)
        self.horizontalLayoutWidget_3.setGeometry(QtCore.QRect(150, 480, 471, 91))
        self.horizontalLayoutWidget_3.setObjectName("horizontalLayoutWidget_3")
        self.uploadLayout = QtWidgets.QHBoxLayout(self.horizontalLayoutWidget_3)
        self.uploadLayout.setContentsMargins(11, 11, 11, 11)
        self.uploadLayout.setSpacing(6)
        self.uploadLayout.setObjectName("uploadLayout")
        self.filepathlabel = QtWidgets.QLabel(self.horizontalLayoutWidget_3)
        font = QtGui.QFont()
        font.setPointSize(20)
        self.filepathlabel.setFont(font)
        self.filepathlabel.setObjectName("filepathlabel")
        self.uploadLayout.addWidget(self.filepathlabel)
        self.filepathEdit = QtWidgets.QLineEdit(self.horizontalLayoutWidget_3)
        self.filepathEdit.setText("")
        self.filepathEdit.setObjectName("filepathEdit")
        self.uploadLayout.addWidget(self.filepathEdit)
        self.selectButton = QtWidgets.QPushButton(self.horizontalLayoutWidget_3)
        self.selectButton.setObjectName("selectButton")
        self.uploadLayout.addWidget(self.selectButton)
        self.storButton = QtWidgets.QPushButton(self.horizontalLayoutWidget_3)
        self.storButton.setObjectName("storButton")
        self.uploadLayout.addWidget(self.storButton)
        self.responselabel = QtWidgets.QLabel(self.centralWidget)
        self.responselabel.setGeometry(QtCore.QRect(160, 110, 791, 31))
        font = QtGui.QFont()
        font.setPointSize(20)
        self.responselabel.setFont(font)
        self.responselabel.setObjectName("responselabel")
        self.pauseButton = QtWidgets.QPushButton(self.centralWidget)
        self.pauseButton.setGeometry(QtCore.QRect(1040, 515, 110, 25))
        self.pauseButton.setObjectName("pauseButton")
        self.transfer_progressBar = QtWidgets.QProgressBar(self.centralWidget)
        self.transfer_progressBar.setGeometry(QtCore.QRect(630, 515, 360, 23))
        self.transfer_progressBar.setProperty("value", 24)
        self.transfer_progressBar.setObjectName("transfer_progressBar")
        self.new_dirButton = QtWidgets.QPushButton(self.centralWidget)
        self.new_dirButton.setGeometry(QtCore.QRect(1010, 250, 150, 31))
        self.new_dirButton.setObjectName("new_dirButton")
        client_gui.setCentralWidget(self.centralWidget)
        self.menuBar = QtWidgets.QMenuBar(client_gui)
        self.menuBar.setGeometry(QtCore.QRect(0, 0, 1280, 22))
        self.menuBar.setObjectName("menuBar")
        client_gui.setMenuBar(self.menuBar)
        self.mainToolBar = QtWidgets.QToolBar(client_gui)
        self.mainToolBar.setObjectName("mainToolBar")
        client_gui.addToolBar(QtCore.Qt.TopToolBarArea, self.mainToolBar)
        self.statusBar = QtWidgets.QStatusBar(client_gui)
        self.statusBar.setObjectName("statusBar")
        client_gui.setStatusBar(self.statusBar)

        self.retranslateUi(client_gui)
        QtCore.QMetaObject.connectSlotsByName(client_gui)

        self.connection_tool = client_connection()
        self.customized_gui()
        self.set_init_slots()


    def retranslateUi(self, client_gui):
        _translate = QtCore.QCoreApplication.translate
        client_gui.setWindowTitle(_translate("client_gui", "client_gui"))
        self.systButton.setText(_translate("client_gui", "SYST"))
        self.command_ip_label.setText(_translate("client_gui", "ip"))
        self.command_ipEdit.setText(_translate("client_gui", "127.0.0.1"))
        self.portlabel.setText(_translate("client_gui", "port"))
        self.command_portEdit.setText(_translate("client_gui", "21"))
        self.usernamelabel.setText(_translate("client_gui", "username"))
        self.usernameEdit.setText(_translate("client_gui", "anonymous"))
        self.passwordlabel.setText(_translate("client_gui", "password"))
        # self.passwordEdit.setText(_translate("client_gui", "ftp"))
        self.choose_serverButton.setText(_translate("client_gui", "local server"))
        self.loginButton.setText(_translate("client_gui", "connect"))
        self.data_iplabel.setText(_translate("client_gui", "ip"))
        self.data_portlabel.setText(_translate("client_gui", "port"))
        self.switchButton.setText(_translate("client_gui", "switch"))
        self.modelabel.setText(_translate("client_gui", "PASV"))
        self.confirmButton.setText(_translate("client_gui", "confirm"))
        self.filepathlabel.setText(_translate("client_gui", "filename"))
        self.selectButton.setText(_translate("client_gui", "select"))
        self.storButton.setText(_translate("client_gui", "upload"))
        self.responselabel.setText(_translate("client_gui", "message"))
        self.pauseButton.setText(_translate("client_gui", "pause"))
        self.new_dirButton.setText(_translate("client_gui", "New Directory"))

    def customized_gui(self):
        self.filelistWidget = my_table_widget(self.centralWidget)
        self.filelistWidget.table_description = 'list_table'
        self.filelistWidget.setGeometry(QtCore.QRect(160, 160, 820, 200))
        self.filelistWidget.setObjectName("filelistWidget")
        self.filelistWidget.setColumnCount(0)
        self.filelistWidget.setRowCount(0)
        self.filelistWidget.horizontalHeader().setStretchLastSection(True)
        self.filelistWidget.horizontalHeader().setDefaultSectionSize(self.filelistWidget.width()/3)
        self.filelistWidget.horizontalHeader().setStyleSheet("QHeaderView::section{background:skyblue;}")
        self.filelistWidget.verticalHeader().setVisible(False)
        self.filelistWidget.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.filelistWidget.setStyleSheet('selection-background-color:lightblue;')
        self.filelistWidget.setColumnCount(3)
        self.filelistWidget.setFrameShape(QFrame.NoFrame)
        self.filelistWidget.setShowGrid(False)
        self.filelistWidget.setEditTriggers(QAbstractItemView.NoEditTriggers)
        divs = ['filename', 'category', 'size']
        self.filelistWidget.setHorizontalHeaderLabels(divs)

        self.transferlistWidget = my_table_widget(self.centralWidget)
        self.transferlistWidget.table_description = 'transfer_table'
        self.transferlistWidget.setGeometry(QtCore.QRect(160, 600, 820, 200))
        self.transferlistWidget.setObjectName("transferlistWidget")
        self.transferlistWidget.setColumnCount(0)
        self.transferlistWidget.setRowCount(0)
        self.transferlistWidget.horizontalHeader().setStretchLastSection(True)
        self.transferlistWidget.horizontalHeader().setDefaultSectionSize(self.filelistWidget.width() / 2)
        self.transferlistWidget.horizontalHeader().setStyleSheet("QHeaderView::section{background:skyblue;}")
        self.transferlistWidget.verticalHeader().setVisible(False)
        self.transferlistWidget.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.transferlistWidget.setStyleSheet('selection-background-color:lightblue;')
        self.transferlistWidget.setColumnCount(2)
        self.transferlistWidget.setFrameShape(QFrame.NoFrame)
        self.transferlistWidget.setShowGrid(False)
        self.transferlistWidget.setEditTriggers(QAbstractItemView.NoEditTriggers)
        file_divs = ['filename', 'status']
        self.transferlistWidget.setHorizontalHeaderLabels(file_divs)


        self.transfer_progressBar.setValue(0)
        self.passwordEdit.setEchoMode(QLineEdit.Password)

    def set_init_slots(self):
        self.switchButton.clicked.connect(self.switch_mode)     # switch data connection mode
        self.mode_signal.connect(self.connection_tool.mode_change)  # tell logic to change mode
        self.loginButton.clicked.connect(self.connect_with_server)  # connect with server and login
        self.login_signal.connect(self.connection_tool.establish_connection)    # send login info
        self.connection_tool.list_sig.connect(self.show_dir_list)   # send LIST info
        self.systButton.clicked.connect(self.connection_tool.syst_request)  # send SYST request
        self.connection_tool.response_sig.connect(self.show_response)   # show response
        self.connection_tool.err_sig.connect(self.show_error)   # report error
        self.confirmButton.clicked.connect(self.confirm_mode)   # confirm mode choice
        self.move_sig.connect(self.connection_tool.move_file)   #rnfr and rnto
        self.new_dirButton.clicked.connect(self.make_directory) # mkd
        self.make_dir_sig.connect(self.connection_tool.make_dir)
        self.selectButton.clicked.connect(self.select_upload_file)  # choose file to stor
        self.storButton.clicked.connect(self.upload_file)   # upload file to server
        self.stor_file_sig.connect(self.connection_tool.upload_file)
        self.retr_file_sig.connect(self.connection_tool.download_file)
        self.connection_tool.transfre_size_sig.connect(self.change_progress)
        self.choose_serverButton.clicked.connect(self.switch_server)
        self.logout_sig.connect(self.connection_tool.shutdown_connection)
        self.rm_file_sig.connect(self.connection_tool.rm_file)
        self.refresh_sig.connect(self.connection_tool.get_dir_info)

        self.pause_sig.connect(self.connection_tool.pause_transfer)
        self.continue_sig.connect(self.connection_tool.continue_transfer)
        self.connection_tool.transfer_finished.connect(self.finish_transfer)

        self.filelistWidget.clicked_dir_sig.connect(self.connection_tool.change_dir)
        self.filelistWidget.cut_action.triggered.connect(self.save_cut_source)
        self.filelistWidget.paste_action.triggered.connect(self.paste_file)
        self.filelistWidget.rename_action.triggered.connect(self.rename_file)
        self.filelistWidget.download_action.triggered.connect(self.download_file)
        self.filelistWidget.delete_action.triggered.connect(self.remove_file)

        self.transferlistWidget.continue_action.triggered.connect(self.chosen_file_continue)


    def connect_with_server(self):
        self.err_msg=''

        self.set_buttons_enabled(True)

        command_ip = self.command_ipEdit.text()
        ip_pattern = r'(\d{1,3}).(\d{1,3}).(\d{1,3}).(\d{1,3})'
        if not re.match(ip_pattern, command_ip):
            QMessageBox.information(self, 'Warning', 'Illegal input', QMessageBox.Ok)

        command_port = int(self.command_portEdit.text())
        username = self.usernameEdit.text()
        password = self.passwordEdit.text()
        self.login_signal.emit(command_ip, command_port, username, password)
        if self.err_msg=='':
            self.loginButton.clicked.disconnect(self.connect_with_server)
            self.loginButton.setText('disconnect')
            self.loginButton.clicked.connect(self.disconnect_with_server)

    def disconnect_with_server(self):
        self.err_msg=''
        self.logout_sig.emit()
        if not self.err_msg=='':
            return
        self.set_buttons_enabled(False)
        self.loginButton.clicked.connect(self.connect_with_server)
        self.loginButton.setText('connect')
        self.loginButton.clicked.disconnect(self.disconnect_with_server)
        self.server_root=''
        self.server_cwd=''


    def confirm_mode(self):
        ip_pattern = r'(\d{1,3}).(\d{1,3}).(\d{1,3}).(\d{1,3})'
        if self.modelabel.text()=='PASV':
            self.mode_signal.emit('PASV', '', 0)
        elif self.modelabel.text()=='PORT':
            data_ip = self.data_ipEdit.text()
            if not re.match(ip_pattern, data_ip):
                QMessageBox.information(self, 'Warning', 'Illegal input', QMessageBox.Ok)
                return
            data_port = int(self.data_portEdit.text())
            if data_port<=0 or data_port>=65536:
                QMessageBox.information(self, 'Warning', 'Illegal input', QMessageBox.Ok)
                return
            self.mode_signal.emit('PORT', data_ip, data_port)

    def switch_mode(self):
        if self.modelabel.text()=='PASV':
            self.modelabel.setText('PORT')
            self.data_ipEdit.setEnabled(True)
            self.data_portEdit.setEnabled(True)

        elif self.modelabel.text()=='PORT':
            self.modelabel.setText('PASV')
            self.data_ipEdit.setEnabled(False)
            self.data_portEdit.setEnabled(False)


    def show_dir_list(self, msg, dir_content):
        self.filelistWidget.delete_items()

        for file_msg in dir_content:
            file_msg.replace('\n', '')
            file_msg.replace('\r', '')
            if file_msg == '':
                continue
            infos = file_msg.split()
            # print(file_msg)

            file_name = QTableWidgetItem()
            file_cat = QTableWidgetItem()
            file_size = QTableWidgetItem()

            if self.server_mode:
                if len(infos) < 9:
                    continue

                file_name.setText(' '.join(infos[8:]))
                if file_name.text()=='':
                    print(file_msg)
                if infos[0].startswith('d'):
                    file_cat.setText('directory')
                else:
                    file_cat.setText('file')
                file_size.setText(infos[4]+ ' B')
            else:

                file_name.setText(' '.join(infos[:-2]))
                file_cat.setText(infos[-2])

                file_size.setText(infos[-1] + ' B')

            row = self.filelistWidget.rowCount()
            self.filelistWidget.insertRow(row)
            self.filelistWidget.setItem(row, 0, file_name)
            self.filelistWidget.setItem(row, 1, file_cat)
            self.filelistWidget.setItem(row, 2, file_size)

        pure_msg = msg.replace('"', '')
        if self.server_root=='':
            self.server_root = pure_msg
        elif pure_msg.startswith(self.server_root) and pure_msg != self.server_root:
            self.filelistWidget.insertRow(0)
            file_name = QTableWidgetItem()
            file_cat = QTableWidgetItem()
            file_size = QTableWidgetItem()
            file_name.setText('..')
            file_cat.setText('directory')
            file_size.setText('')
            self.filelistWidget.setItem(0, 0, file_name)
            self.filelistWidget.setItem(0, 1, file_cat)
            self.filelistWidget.setItem(0, 2, file_size)

        self.server_cwd = pure_msg
        self.responselabel.setText(pure_msg)

    def save_cut_source(self):
        self.rnfr_source = self.server_cwd+'/'+self.filelistWidget.clicked_file_name

    def paste_file(self):
        self.rnto_target = self.server_cwd+'/'+self.rnfr_source.split('/')[-1]
        self.move_sig.emit(self.rnfr_source, self.rnto_target)
        self.rnfr_source = ''
        self.rnto_target = ''

    def rename_file(self):
        self.rnfr_source = self.filelistWidget.clicked_file_name
        new_name, _ = QInputDialog.getText(self,'rename', 'new name:', QLineEdit.Normal, '')
        self.rnto_target = new_name
        self.move_sig.emit(self.rnfr_source, self.rnto_target)
        self.rnfr_source = ''
        self.rnto_target = ''

    def upload_file(self):
        file_name = self.filepathEdit.text()
        file_size = os.path.getsize(file_name)

        self.connection_tool.paused = False
        self.err_msg=''
        self.stor_file_sig.emit(file_name)
        if not self.err_msg=='':
            return
        self.pauseButton.setEnabled(True)
        self.pauseButton.clicked.connect(self.pause_transfer)
        self.transfer_progressBar.setMaximum(file_size)
        self.transfer_progressBar.setValue(0)
        if file_size == 0:
            self.transfer_progressBar.setMaximum(1)
            self.transfer_progressBar.setValue(1)

        row = self.transferlistWidget.rowCount()
        self.transferlistWidget.insertRow(row)
        name_item = QTableWidgetItem()
        status_item = QTableWidgetItem()
        name_item.setText(file_name)
        status_item.setText('uploading')
        self.transferlistWidget.setItem(row, 0, name_item)
        self.transferlistWidget.setItem(row, 1, status_item)
        self.transferlistWidget.row_of_file = row

        transfer_record = {'name': file_name,
                           'offset': 0,
                           'size': file_size,
                           'transfer': 'upload',
                           'status': 'transfering'}
        self.transfer_list.append(transfer_record)

    def download_file(self):
        file_name = self.filelistWidget.clicked_file_name
        file_size = self.filelistWidget.clicked_file_size

        self.connection_tool.paused = False
        self.err_msg = ''
        self.retr_file_sig.emit(file_name)
        if not self.err_msg=='':
            return
        self.transfer_progressBar.setMaximum(file_size)
        self.transfer_progressBar.setValue(0)
        if file_size == 0:
            self.transfer_progressBar.setMaximum(1)
            self.transfer_progressBar.setValue(1)
        row = self.transferlistWidget.rowCount()
        self.transferlistWidget.insertRow(row)
        name_item = QTableWidgetItem()
        status_item = QTableWidgetItem()
        name_item.setText(self.server_cwd + '/' + file_name)
        status_item.setText('downloading')
        self.transferlistWidget.setItem(row, 0, name_item)
        self.transferlistWidget.setItem(row, 1, status_item)
        self.transferlistWidget.row_of_file = row

        transfer_record = {'name': self.server_cwd + '/' + file_name,
                           'offset': 0,
                           'size': file_size,
                           'transfer': 'download',
                           'status': 'transfering'}
        self.transfer_list.append(transfer_record)
        self.pauseButton.setEnabled(True)
        self.pauseButton.clicked.connect(self.pause_transfer)


    def make_directory(self):
        new_dir, _ = QInputDialog.getText(self, 'New Folder', 'folder name:', QLineEdit.Normal, '')
        self.make_dir_sig.emit(new_dir)

    def remove_file(self):
        file_to_remove = self.filelistWidget.clicked_file_name
        self.rm_file_sig.emit(file_to_remove)

    def select_upload_file(self):
        complete_file_name, file_type = QFileDialog.getOpenFileName(self, 'select file', './', 'All Files (*)')
        # file_name = complete_file_name.split('/')[-1]
        self.filepathEdit.setText(complete_file_name)

    def pause_transfer(self):
        self.pauseButton.clicked.disconnect(self.pause_transfer)
        # self.pauseButton.clicked.connect(self.continue_transfer)
        # self.pauseButton.setText('continue')
        self.pauseButton.setEnabled(False)

        self.pause_sig.emit()

        row = self.transferlistWidget.row_of_file
        status_item = self.transferlistWidget.itemFromIndex(self.transferlistWidget.model().index(row, 1))
        name_item = self.transferlistWidget.itemFromIndex(self.transferlistWidget.model().index(row, 0))
        file_name = name_item.text()
        status_item.setText('paused')
        # self.transferlistWidget.setItem(row, 1, status_item)
        for single_record in self.transfer_list:
            if single_record['name']==file_name:
                single_record['status']='paused'
                single_record['offset']=self.connection_tool.rw_offset
                break



    def continue_transfer(self):
        self.pauseButton.clicked.disconnect(self.continue_transfer)
        self.pauseButton.clicked.connect(self.pause_transfer)
        self.pauseButton.setText('pause')
        self.continue_sig.emit()

    def chosen_file_continue(self):
        # self.pauseButton.clicked.disconnect(self.continue_transfer)
        if self.connection_tool.is_transfering:
            return
        self.pauseButton.setEnabled(True)
        self.pauseButton.clicked.connect(self.pause_transfer)
        self.pauseButton.setText('pause')

        row = self.transferlistWidget.clicked_row
        status_item = self.transferlistWidget.itemFromIndex(self.transferlistWidget.model().index(row, 1))
        name_item = self.transferlistWidget.itemFromIndex(self.transferlistWidget.model().index(row, 0))
        file_name = name_item.text()
        for single_record in self.transfer_list:
            if single_record['name']==file_name:
                single_record['status']='transfering'
                self.connection_tool.rw_offset=single_record['offset']
                self.transfer_progressBar.setMaximum(single_record['size'])
                self.transfer_progressBar.setValue(single_record['offset'])
                status_item.setText(single_record['transfer']+'ing')
                if single_record['transfer']=='upload':
                    self.connection_tool.transfering_file = file_name
                    self.connection_tool.retr_or_stor = True
                elif single_record['transfer']=='download':
                    self.connection_tool.transfering_file = file_name.split('/')[-1]
                    self.connection_tool.retr_or_stor = False
                break
        # self.transferlistWidget.setItem(row, 1, status_item)
        self.transferlistWidget.row_of_file = row

        self.continue_sig.emit()

    def finish_transfer(self):
        self.pauseButton.clicked.disconnect(self.pause_transfer)

        row = self.transferlistWidget.row_of_file
        status_item = self.transferlistWidget.itemFromIndex(self.transferlistWidget.model().index(row, 1))
        name_item = self.transferlistWidget.itemFromIndex(self.transferlistWidget.model().index(row, 0))
        file_name = name_item.text()
        for single_record in self.transfer_list:
            if single_record['name'] == file_name:
                single_record['status'] = 'finished'
                break
        status_item.setText('finished')
        self.refresh_sig.emit()
        # self.transferlistWidget.setItem(row, 1, status_item)


    def switch_server(self):
        current_server = self.choose_serverButton.text()
        if current_server=='local server':
            self.server_mode = True
            self.choose_serverButton.setText('standard server')
        elif current_server=='standard server':
            self.server_mode = False
            self.choose_serverButton.setText('local server')

    def show_response(self, msg):
        self.responselabel.setText(msg)

    def show_error(self, msg):
        QMessageBox.information(self, 'Warning', msg, QMessageBox.Ok)
        self.err_msg=msg

    def change_progress(self, transfer_size):
        self.transfer_progressBar.setValue(transfer_size)

    def set_buttons_enabled(self, choice):
        self.systButton.setEnabled(choice)
        self.new_dirButton.setEnabled(choice)
        self.selectButton.setEnabled(choice)
        self.storButton.setEnabled(choice)
        self.pauseButton.setEnabled(choice)
        self.filelistWidget.setEnabled(choice)

