from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *
from PyQt5.QtCore import *

class my_table_widget(QTableWidget):
    clicked_dir_sig = pyqtSignal(str)
    cwd_sig = pyqtSignal(str)

    clicked_file_name = ''
    clicked_file_cat = ''
    clicked_file_size = 0
    table_description = ''
    clicked_row = 0
    row_of_file = 0
    def __init__(self, parent):
        super().__init__(parent)
        self.createActions()

    def createActions(self):
        self.op_menu = QMenu()
        self.delete_action = QAction()
        self.rename_action = QAction()
        self.cut_action = QAction()
        self.paste_action = QAction()
        self.download_action = QAction()
        self.continue_action = QAction()

        self.delete_action.setText('delete')
        self.rename_action.setText('rename')
        self.cut_action.setText('cut')
        self.paste_action.setText('paste')
        self.download_action.setText('download')
        self.continue_action.setText('continue')

    def contextMenuEvent(self, QContextMenuEvent):
        self.op_menu.clear()
        click_point = QContextMenuEvent.pos()
        item_clicked = self.itemAt(click_point)
        if item_clicked:
            if self.table_description=='list_table':
                self.op_menu.addAction(self.delete_action)
                self.op_menu.addAction(self.rename_action)
                self.op_menu.addSeparator()
                self.op_menu.addAction(self.cut_action)
                self.op_menu.addAction(self.paste_action)
                self.op_menu.addSeparator()
                row = item_clicked.row()
                file_cat = self.itemFromIndex(self.model().index(row, 1)).text()
                if file_cat=='file':
                    self.op_menu.addAction(self.download_action)
                file_name = self.itemFromIndex(self.model().index(row, 0)).text()
                file_size = int(self.itemFromIndex(self.model().index(row, 2)).text().split()[0])
                self.clicked_file_cat = file_cat
                self.clicked_file_name = file_name
                self.clicked_file_size = file_size
                self.op_menu.exec(QCursor.pos())
                QContextMenuEvent.accept()
            elif self.table_description=='transfer_table':
                row = item_clicked.row()
                self.clicked_row = row
                file_status = self.itemFromIndex(self.model().index(row, 1)).text()
                self.clicked_file_name = self.itemFromIndex(self.model().index(row, 0)).text()
                if file_status=='paused':
                    self.op_menu.addAction(self.continue_action)
                self.op_menu.exec(QCursor.pos())
                QContextMenuEvent.accept()


    def mouseDoubleClickEvent(self, QMouseEvent):
        click_point = QMouseEvent.pos()
        item_clicked = self.itemAt(click_point)
        if item_clicked:
            row = item_clicked.row()
            file_cat = self.itemFromIndex(self.model().index(row, 1)).text()
            if file_cat == 'directory':
                file_name = self.itemFromIndex(self.model().index(row, 0)).text()
                self.clicked_dir_sig.emit(file_name)

    def delete_items(self):
        for i in range(self.rowCount()):
            self.removeRow(0)