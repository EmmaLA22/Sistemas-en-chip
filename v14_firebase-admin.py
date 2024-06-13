# v14_firebase-admin.py

import sys
import os
import firebase_admin
from firebase_admin import credentials, db
import pygame
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QHBoxLayout, QLabel, QSlider, QPushButton, QListWidget, QListWidgetItem, QGridLayout
from PyQt5.QtCore import Qt, QTimer, QThread, pyqtSignal
from PyQt5.QtGui import QPixmap
import time

from luma.core.interface.serial import i2c
from luma.oled.device import ssd1306
from PIL import Image, ImageDraw, ImageFont

# Configuración de la pantalla OLED
serial = i2c(port=1, address=0x3C)
device = ssd1306(serial, width=128, height=64)

# Directorio del trabajo fijo
os.chdir("/home/equipodinamita/Desktop/Actividades/Rock")

class FirebaseThread(QThread):
    status_signal = pyqtSignal(int)

    def __init__(self):
        super().__init__()

    def run(self):
        while True:
            ref = db.reference('/embot1')
            status = ref.child('status').get()
            self.status_signal.emit(status)
            time.sleep(5)

# Configuración de Firebase
cred = credentials.Certificate("/home/equipodinamita/Desktop/Actividades/sistemas-en-chips-firebase-adminsdk-7ljf6-97834ed25a.json")
firebase_admin.initialize_app(cred, {
    #'databaseURL': 'https://mbot-pruebas-default-rtdb.firebaseio.com' #Erik
    'databaseURL': 'https://sistemas-en-chips-default-rtdb.firebaseio.com' #Emma
})

connected = False

while not connected:
    try:
        connected = True
        print("Conexion exitosa")
    except Exception as e:
        print("Error al conectar la base de datos")
        print("\nReintentando...")
        time.sleep(3)

class Interfaz(QWidget):
    def __init__(self):
        super().__init__()

        """
        firebase_timer = QTimer()
        firebase_timer.timeout.connect(self.readFirebase)
        firebase_timer.start(5000)
        """

        # Crear y conectar el hilo de Firebase
        self.firebase_thread = FirebaseThread()
        self.firebase_thread.status_signal.connect(self.update_status)
        self.firebase_thread.start()

        self.setWindowTitle('Reproductor de Música con Control de Embot')
        self.setGeometry(200, 200, 1000, 600)

        pygame.init()
        pygame.mixer.init()
        pygame.mixer.music.set_endevent(pygame.USEREVENT)

        self.setStyleSheet("""
            background-color: #595453; color: #0C53F3;
            QPushButton {
                background-color: #C6CACC; color: #19275B; border: 2px solid #3A11B5; border-radius: 10px; padding: 10px;
            }
            QPushButton:hover {
                background-color: #3A11B5; color: #3A11B5;
            }
            QSlider::groove:horizontal {
                background: #000000;
                height: 8px;
            }
            QSlider::handle:horizontal {
                background: #3B3535;
                border: 1px solid #3B3535;
                width: 18px;
                margin: -5px 0; 
                border-radius: 3px;
            }
        """)

        main_layout = QHBoxLayout()

        column1 = QVBoxLayout()
        column2 = QVBoxLayout()

        self.current_song_frame = QLabel(self)
        self.current_song_frame.setFixedSize(400, 50)
        self.current_song_frame.setStyleSheet("background-color: #424747; color: #39ff14; border: 2px solid #3A11B5; border-radius: 10px; padding: 10px;")
        column1.addWidget(self.current_song_frame)
        column1.setAlignment(self.current_song_frame, Qt.AlignTop)

        self.progress_slider = QSlider(Qt.Horizontal, self)
        self.progress_slider.setFixedSize(400, 10)
        column1.addWidget(self.progress_slider)

        playback_buttons_layout = QHBoxLayout()
        
        self.button_prev = QPushButton('Anterior', self)
        self.button_prev.setStyleSheet("background-color : #CCCCCC")
        self.button_prev.clicked.connect(self.previous_song)
        playback_buttons_layout.addWidget(self.button_prev)
        
        self.button_play_pause = QPushButton('|>', self)
        self.button_play_pause.setStyleSheet("background-color : #CCCCCC")
        self.button_play_pause.clicked.connect(self.play_pause_song)
        playback_buttons_layout.addWidget(self.button_play_pause)
        
        self.button_next = QPushButton('Siguiente', self)
        self.button_next.setStyleSheet("background-color : #CCCCCC")
        self.button_next.clicked.connect(self.next_song)
        playback_buttons_layout.addWidget(self.button_next)
        
        column1.addLayout(playback_buttons_layout)
        column1.setAlignment(playback_buttons_layout, Qt.AlignCenter)

        volume_label = QLabel('Volumen', self)
        volume_label.setAlignment(Qt.AlignCenter)
        column1.addWidget(volume_label)

        self.volume_slider = QSlider(Qt.Horizontal, self)
        self.volume_slider.setMinimum(0)
        self.volume_slider.setMaximum(100)
        self.volume_slider.setValue(50)
        self.volume_slider.valueChanged.connect(self.change_volume)
        column1.addWidget(self.volume_slider)
        column1.setAlignment(self.volume_slider, Qt.AlignCenter)

        self.song_list = QListWidget(self)
        self.load_songs()
        self.song_list.setFixedSize(400, 250)
        self.song_list.setStyleSheet("background-color: #424747; color: #39ff14; border: 2px solid #3A11B5; border-radius: 10px; padding: 10px;")
        column1.addWidget(self.song_list)
        column1.setAlignment(self.song_list, Qt.AlignBottom)

        self.image_label = QLabel(self)
        pixmap = QPixmap('/home/equipodinamita/Desktop/Actividades/Rock/mbot_Mega.png')
        self.image_label.setPixmap(pixmap)
        self.image_label.setGeometry(0, 0, pixmap.width(), pixmap.height())
        self.image_label.setAlignment(Qt.AlignCenter)
        column2.addWidget(self.image_label)

        buttons_layout = QGridLayout()

        self.button_up = QPushButton('Adelante', self)
        self.button_up.setStyleSheet("background-color : #CCCCCC")
        self.button_up.clicked.connect(self.mBot_Forward)  # Conectar el botón con el método
        buttons_layout.addWidget(self.button_up, 0, 1)

        self.button_left = QPushButton('Izquierda', self)
        self.button_left.setStyleSheet("background-color : #CCCCCC")
        self.button_left.clicked.connect(self.mBot_Left)  # Conectar el botón con el método
        buttons_layout.addWidget(self.button_left, 1, 0)

        self.button_right = QPushButton('Derecha', self)
        self.button_right.setStyleSheet("background-color : #CCCCCC")
        self.button_right.clicked.connect(self.mBot_Right)  # Conectar el botón con el método
        buttons_layout.addWidget(self.button_right, 1, 2)

        self.button_down = QPushButton('Atrás', self)
        self.button_down.setStyleSheet("background-color : #CCCCCC")
        self.button_down.clicked.connect(self.mBot_Backward)  # Conectar el botón con el método
        buttons_layout.addWidget(self.button_down, 2, 1)

        column2.addLayout(buttons_layout)
        column2.setAlignment(buttons_layout, Qt.AlignCenter)

        main_layout.addLayout(column1)
        main_layout.addLayout(column2)

        self.setLayout(main_layout)

        self.scroll_position = 0
        self.timer = QTimer()
        self.timer.timeout.connect(self.scroll_text)
        self.timer.start(200) # Ajustar este valor si se alenta

        self.timer_progress = QTimer()
        self.timer_progress.timeout.connect(self.update_progress)
        self.timer_progress.start(1000) # 1 segundo es una buena frecuencia para actualizar el progreso

        self.init_oled_display()

    def init_oled_display(self):
        self.font = ImageFont.load_default()
        self.image = Image.new('1', (device.width, device.height))
        self.draw = ImageDraw.Draw(self.image)
        self.update_oled_display("Iniciando...", 0, 1)

    def update_oled_display(self, song_name, progress, total_length):
        self.draw.rectangle((0, 0, device.width, device.height), outline=0, fill=0)

        if len(song_name) > 13:
            line1 = song_name[:13]
            line2 = song_name[13:]
            self.draw.text((0, 0), f"Canción:\n {line1}", font=self.font, fill=255)
            self.draw.text((0, 25), f"{line2}", font=self.font, fill=255)
        else:
            self.draw.text((0, 0), f"Canción:\n {song_name}", font=self.font, fill=255)

        progress_percentage = int((progress / total_length) * 100) if total_length > 0 else 0
        slider_top = 40
        slider_bottom = slider_top + 10
        self.draw.rectangle((0, slider_top, progress_percentage, slider_bottom), outline=1, fill=1)

        device.display(self.image)

    def update_status(self, status):
        if status == 1:
            self.button_up.setStyleSheet("background-color : #00FF00")
        else:
            self.button_up.setStyleSheet("background-color : #FF0000")


    def load_songs(self):
        songs_folder = "/home/equipodinamita/Desktop/Actividades/Rock"
        if os.path.isdir(songs_folder):
            songs = [f for f in os.listdir(songs_folder) if f.endswith('.mp3')]
            for song in songs:
                item = QListWidgetItem(song)
                self.song_list.addItem(item)

    def play_pause_song(self):
        if pygame.mixer.music.get_busy():
            pygame.mixer.music.pause()
            self.button_play_pause.setText('|>')
            self.current_song_frame.setText('Canción actual: Pausada')
            self.update_oled_display('Pausada', 0, 1)
        else:
            self.play_song()
            self.button_play_pause.setText('||')

    """
    def play_song(self):
        current_song = self.song_list.currentItem().text() if self.song_list.currentItem() else "Ninguna"
        if current_song != "Ninguna":
            song_path = os.path.join("/home/equipodinamita/Desktop/Actividades/Rock", current_song)
            pygame.mixer.music.load(song_path)
            pygame.mixer.music.set_endevent(pygame.USEREVENT)
            pygame.mixer.music.play()
            self.current_song_frame.setText(f'Canción actual: {current_song}')
            self.update_oled_display(current_song, 0, 1)
    """

    def play_song(self):
        current_song = self.song_list.currentItem().text() if self.song_list.currentItem() else "Ninguna"
        if current_song != "Ninguna":
            song_path = os.path.join("/home/equipodinamita/Desktop/Actividades/Rock", current_song)
            # Imprimir la ruta para depuración
            print(f"Intentando cargar la canción desde la ruta: {song_path}")
            if os.path.exists(song_path):
                try:
                    pygame.mixer.music.load(song_path)
                    pygame.mixer.music.set_endevent(pygame.USEREVENT)
                    pygame.mixer.music.play()
                    self.current_song_frame.setText(f'Canción actual: {current_song}')
                    self.update_oled_display(current_song, 0, 1)
                except Exception as e:
                    print(f"Error al cargar la canción '{song_path}': {e}")
            else:
                print(f"El archivo '{song_path}' no se encuentra.")


    def previous_song(self):
        current_row = self.song_list.currentRow()
        if current_row > 0:
            self.song_list.setCurrentRow(current_row - 1)
            self.play_song()

    def next_song(self):
        current_row = self.song_list.currentRow()
        if current_row < self.song_list.count() - 1:
            self.song_list.setCurrentRow(current_row + 1)
            self.play_song()

    def change_volume(self):
        volume = self.volume_slider.value() / 100
        pygame.mixer.music.set_volume(volume)

    def scroll_text(self):
        current_text = self.current_song_frame.text()
        if len(current_text) > 25:
            self.scroll_position = (self.scroll_position + 1) % len(current_text)
            display_text = current_text[self.scroll_position:] + current_text[:self.scroll_position]
            self.current_song_frame.setText(display_text)

    # ++++ Métodos para mover el mBot +++++++++++++++++
    def mBot_Forward(self):
        db.reference("test").update({"Serial2": "adelante"})
        print("Enviando comando 'adelante' a Firebase")

    def mBot_Backward(self):
        db.reference("test").update({"Serial2": "atras"})
        print("Enviando comando 'atras' a Firebase")

    def mBot_Left(self):
        db.reference("test").update({"Serial2": "izquierda"})
        print("Enviando comando 'izquierda' a Firebase")

    def mBot_Right(self):
        db.reference("test").update({"Serial2": "derecha"})
        print("Enviando comando 'derecha' a Firebase")
    #----------------------------------------------------------------------------

    """
    def update_progress(self):
        if pygame.mixer.music.get_busy():
            current_time = pygame.mixer.music.get_pos() / 1000
            total_length = pygame.mixer.Sound(self.song_list.currentItem().text()).get_length() if self.song_list.currentItem() else 1
            self.progress_slider.setValue(int((current_time / total_length) * 100))
            self.update_oled_display(self.song_list.currentItem().text(), current_time, total_length)
    """

    def update_progress(self):
        if pygame.mixer.music.get_busy():
            current_time = pygame.mixer.music.get_pos() / 1000
            current_song_item = self.song_list.currentItem()
        
            if current_song_item:
             song_path = os.path.join("/home/equipodinamita/Desktop/Actividades/Rock", current_song_item.text())
             if os.path.exists(song_path):
                  total_length = pygame.mixer.Sound(song_path).get_length()
                  self.progress_slider.setValue(int((current_time / total_length) * 100))
                  self.update_oled_display(current_song_item.text(), current_time, total_length)
            else:
                print(f"El archivo '{song_path}' no se encuentra.")

    """        
    def readFirebase(self):
        ref = db.reference('/embot1')
        status = ref.child('status').get()

        if status == 1:
            self.button_up.setStyleSheet("background-color : #00FF00")
        else:
            self.button_up.setStyleSheet("background-color : #FF0000")
    """

if __name__ == "__main__":
    app = QApplication(sys.argv)
    interfaz = Interfaz()
    interfaz.show()
    sys.exit(app.exec_())