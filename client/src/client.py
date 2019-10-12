from client_gui import *
from PyQt5 import QtGui

import sys

if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    client_gui = QtWidgets.QMainWindow()
    client_gui.setWindowIcon(QIcon('ftp.png'))
    ui = Ui_client_gui()
    ui.setupUi(client_gui)
    client_gui.show()
    sys.exit(app.exec_())