#!/usr/bin/env python

from PyQt4 import QtGui
from PyQt4.QtCore import QTimer
import sys
import time
import user_frontend
import usrp_dab_rx
import usrp_dab_tx
import math
import json
import sip
import os

class DABstep(QtGui.QMainWindow, user_frontend.Ui_MainWindow):
    def __init__(self, parent=None):
        super(DABstep, self).__init__(parent)
        self.setupUi(self)

        # show logo if it exists
        if os.path.exists("DAB_logo.png"):
            self.label_logo.setText("<img src=\"DAB_logo.png\">")

        # tab definitions
        self.modes = {"rec": 0, "trans": 1, "dev": 2}
        self.mode = self.modes["rec"]
        self.mode_tabs.currentChanged.connect(self.change_tab)

        # lookup table
        self.table = lookup_tables()

        ######################################################################
        # TAB RECEPTION (defining variables, signals and slots)
        ######################################################################
        # receiver variables
        self.frequency = 208.064e6
        self.bit_rate = 8
        self.address = 0
        self.size = 6
        self.protection = 2
        self.volume = 80
        self.subch = -1
        self.need_new_init = True
        self.recorder = False
        self.file_path = "None"
        self.src_is_USRP = True

        # table preparations
        header = self.table_mci.horizontalHeader()
        header.setResizeMode(0, QtGui.QHeaderView.ResizeToContents)
        header.setResizeMode(1, QtGui.QHeaderView.Stretch)
        header.setResizeMode(2, QtGui.QHeaderView.ResizeToContents)
        header.setResizeMode(3, QtGui.QHeaderView.ResizeToContents)
        header.setStretchLastSection(False)
        self.table_mci.verticalHeader().hide()

        # timer for update of SNR
        self.snr_timer = QTimer()
        self.snr_timer.timeout.connect(self.snr_update)
        # change of source by radio buttons
        self.rbtn_USRP.clicked.connect(self.src2USRP)
        self.rbtn_File.clicked.connect(self.src2File)
        # set file path
        self.btn_file_path.clicked.connect(self.set_file_path)
        # init button initializes receiver with center frequency
        self.btn_init.clicked.connect(self.update_statusBar)
        self.btn_init.clicked.connect(self.init_receiver)
        # update button updates services in table
        self.btn_update_info.clicked.connect(self.update_service_info)
        # a click on a cell of the service table let the text box show more info to the selected service
        self.table_mci.cellClicked.connect(self.selected_subch)
        # a click on the play button compiles receiver with new subch info and plays the audio
        self.btn_play.clicked.connect(self.play_audio)
        # stop button click stops audio reception
        self.btn_stop.clicked.connect(self.stop_reception)
        # volume slider moved
        self.slider_volume.valueChanged.connect(self.set_volume)
        # record button
        self.btn_record.clicked.connect(self.record_audio)

        ######################################################################
        # DEVELOPER MODE
        ######################################################################
        self.dev_mode_active = False
        # hide close button and just show open button
        self.btn_dev_mode_close.hide()
        # dev mode open button pressed
        self.btn_dev_mode_open.clicked.connect(self.dev_mode_open)
        # dev mode close button pressed
        self.btn_dev_mode_close.clicked.connect(self.dev_mode_close)
        # firecode display
        self.snr_timer.timeout.connect(self.update_firecode)
        self.label_firecode.setText("")
        font = QtGui.QFont()
        font.setFamily("CourierNew")
        self.label_firecode.setFont(font)
        self.label_firecode.hide()
        self.content_count = 0



        ######################################################################
        # TAB TRANSMISSION (defining variables, signals and slots)
        ######################################################################
        # change of sink by radio buttons
        self.t_rbtn_USRP.clicked.connect(self.t_set_sink_USRP)
        self.t_rbtn_File.clicked.connect(self.t_set_sink_File)
        # create dict for service components
        self.components = [
            {"label": self.t_label_comp1, "data_rate_label": self.t_label_rate1, "data_rate": self.t_spin_rate1,
             "protection_label": self.t_label_prot1, "protection": self.t_spin_prot1, "enabled": True,
             "src_label": self.t_label_comp_src1, "src_path_disp": self.t_label_path_src1,
             "src_btn": self.t_btn_path_src1, "src_path":"None"},
            {"label": self.t_label_comp2, "data_rate_label": self.t_label_rate2, "data_rate": self.t_spin_rate2,
             "protection_label": self.t_label_prot2, "protection": self.t_spin_prot2, "enabled": False,
             "src_label": self.t_label_comp_src2, "src_path_disp": self.t_label_path_src2,
             "src_btn": self.t_btn_path_src2, "src_path":"None"},
            {"label": self.t_label_comp3, "data_rate_label": self.t_label_rate3, "data_rate": self.t_spin_rate3,
             "protection_label": self.t_label_prot3, "protection": self.t_spin_prot3, "enabled": False,
             "src_label": self.t_label_comp_src3, "src_path_disp": self.t_label_path_src3,
             "src_btn": self.t_btn_path_src3, "src_path":"None"},
            {"label": self.t_label_comp4, "data_rate_label": self.t_label_rate4, "data_rate": self.t_spin_rate4,
             "protection_label": self.t_label_prot4, "protection": self.t_spin_prot4, "enabled": False,
             "src_label": self.t_label_comp_src4, "src_path_disp": self.t_label_path_src4,
             "src_btn": self.t_btn_path_src4, "src_path":"None"},
            {"label": self.t_label_comp5, "data_rate_label": self.t_label_rate5, "data_rate": self.t_spin_rate5,
             "protection_label": self.t_label_prot5, "protection": self.t_spin_prot5, "enabled": False,
             "src_label": self.t_label_comp_src5, "src_path_disp": self.t_label_path_src5,
             "src_btn": self.t_btn_path_src5, "src_path":"None"},
            {"label": self.t_label_comp6, "data_rate_label": self.t_label_rate6, "data_rate": self.t_spin_rate6,
             "protection_label": self.t_label_prot6, "protection": self.t_spin_prot6, "enabled": False,
             "src_label": self.t_label_comp_src6, "src_path_disp": self.t_label_path_src6,
             "src_btn": self.t_btn_path_src6, "src_path":"None"},
            {"label": self.t_label_comp7, "data_rate_label": self.t_label_rate7, "data_rate": self.t_spin_rate7,
             "protection_label": self.t_label_prot7, "protection": self.t_spin_prot7, "enabled": False,
             "src_label": self.t_label_comp_src7, "src_path_disp": self.t_label_path_src7,
             "src_btn": self.t_btn_path_src7, "src_path":"None"}]
        # update service components initially to hide the service components 2-7
        self.t_update_service_components()
        # provide suggestions for language combo box
        for i in range(0,len(self.table.languages)):
            self.t_combo_language.addItem(self.table.languages[i])
        # provide suggestions for country combo box
        for i in range(0,len(self.table.country_ID_ECC_E0)):
            self.t_combo_country.addItem(self.table.country_ID_ECC_E0[i])
        # update dict "components" and display of service components if spinbox "number of channels" is changed
        self.t_spin_num_subch.valueChanged.connect(self.t_change_num_subch)
        # write ensemble/service info when init is pressed
        self.t_btn_init.pressed.connect(self.t_init_transmitter)
        # start flowgraph when play btn pressed
        self.t_btn_play.pressed.connect(self.t_run_transmitter)
        # stop button pressed
        self.t_btn_stop.pressed.connect(self.t_stop_transmitter)
        # path for File sink path
        self.t_btn_file_path.pressed.connect(self.t_set_file_path)
        # path selection for all 7 (possible) sub channels
        self.t_btn_path_src1.pressed.connect(self.t_set_subch_path1)
        self.t_btn_path_src2.pressed.connect(self.t_set_subch_path2)
        self.t_btn_path_src3.pressed.connect(self.t_set_subch_path3)
        self.t_btn_path_src4.pressed.connect(self.t_set_subch_path4)
        self.t_btn_path_src5.pressed.connect(self.t_set_subch_path5)
        self.t_btn_path_src6.pressed.connect(self.t_set_subch_path6)
        self.t_btn_path_src7.pressed.connect(self.t_set_subch_path7)
        # set volume if volume slider is changed
        self.t_slider_volume.valueChanged.connect(self.t_set_volume)

    ################################
    # general functions
    ################################
    def change_tab(self):
        if self.mode_tabs.currentWidget() is self.tab_transmission:
            print "changed to transmission mode"
            self.t_update_service_components()

        elif self.mode_tabs.currentWidget() is self.tab_reception:
            print "changed to reception mode"
        else:
            print "changed to unknown tab"

    ################################
    # Receiver functions
    ################################

    def src2USRP(self):
        # enable/disable buttons
        self.btn_file_path.setEnabled(False)
        self.label_path.setEnabled(False)
        self.spinbox_frequency.setEnabled(True)
        self.label_frequency.setEnabled(True)
        self.src_is_USRP = True

    def src2File(self):
        # enable/disable buttons
        self.btn_file_path.setEnabled(True)
        self.label_path.setEnabled(True)
        self.spinbox_frequency.setEnabled(False)
        self.label_frequency.setEnabled(False)
        self.src_is_USRP = False

    def set_file_path(self):
        path = QtGui.QFileDialog.getOpenFileName(self, "Pick a file with recorded IQ samples")
        if path:  # if user didn't pick a directory don't continue
            self.file_path = str(path)
            self.label_path.setText(path)

    def init_receiver(self):
        # stop any processes that access to an instance of usrp_dab_rx
        self.snr_timer.stop()
        # set up and start flowgraph
        self.my_receiver = usrp_dab_rx.usrp_dab_rx(self.spinbox_frequency.value(), self.bit_rate, self.address, self.size, self.protection,
                                          self.src_is_USRP, self.file_path, self.recorder)
        self.my_receiver.start()
        # status bar
        self.statusBar.showMessage("initialization finished!")
        # init dev mode
        self.dev_mode_init()
        # once scan ensemble automatically (after per clicking btn)
        time.sleep(1)
        self.update_service_info()
        self.btn_update_info.setEnabled(True)
        self.snr_update()
        self.snr_timer.start(500)

    def update_statusBar(self):
        self.statusBar.showMessage("initializing receiver ...")

    def update_service_info(self):
        # set status bar message
        self.statusBar.showMessage("scanning ensemble...")
        # remove all old data from table at first
        while (self.table_mci.rowCount() > 0):
            self.table_mci.removeRow(0)

        # get new data from fic_sink
        service_data = self.get_service_info()
        service_labels = self.get_service_labels()

        # write new data to table
        for n, key in enumerate(sorted(service_data, key=lambda service_data: service_data['ID'])):
            # add a new row
            self.table_mci.insertRow(n)
            # print ID in first collumn
            self.table_mci.setItem(n, 0, QtGui.QTableWidgetItem(str(key['ID'])))
            # print reference (later label)
            self.table_mci.setItem(n, 1, QtGui.QTableWidgetItem(next((item for item in service_labels if item["reference"] == key['reference']), {'label':"not found"})['label']))
            # print type
            self.table_mci.setItem(n, 3, QtGui.QTableWidgetItem("primary" if key['primary'] == True else "secondary"))
            # print DAB Mode
            self.table_mci.setItem(n, 2, QtGui.QTableWidgetItem(("DAB+" if key['DAB+'] == True else "DAB ")))

        # set number of sub channels
        self.num_subch = self.table_mci.rowCount()

        # display ensemble info
        ensemble_data = self.get_ensemble_info()
        self.label_ensemble.setText(ensemble_data.keys()[0].strip())
        self.label_country.setText(str(self.table.country_ID_ECC_E0[int(ensemble_data.values()[0]['country_ID'])]))
        self.lcd_number_num_subch.display(self.num_subch)
        # status bar
        self.statusBar.showMessage("Select a Service Component.")

    def selected_subch(self):
        # enable/disable buttons
        self.btn_play.setEnabled(True)
        self.btn_record.setEnabled(True)
        # new subch was selected
        self.btn_play.setText("Play")
        self.slider_volume.setEnabled(False)

        # get selected sub-channel by its ID
        ID = self.table_mci.item(self.table_mci.currentRow(), 0).text()
        # find related service reference to ID
        reference = next((item for item in self.get_service_info() if item['ID'] == int(ID)), {'reference':-1})['reference']
        # get dicts to specific service and sub-channel
        service_data = next((item for item in self.get_service_info() if item['ID'] == int(ID)), {"reference":-1, "ID": -1, "primary": True})
        service_label = next((item for item in self.get_service_labels() if item['reference'] == int(reference)), {"reference": -1, "label":"not found"})
        subch_data = next((item for item in self.get_subch_info() if item['ID'] == int(ID)), {"ID":-1, "address":0, "protection":0,"size":0})
        #programme_type = next((item for item in self.get_programme_type() if item["reference"] == reference), {"programme_type":0,"language":0})

        # update sub-channel info for receiver
        self.address = int(subch_data['address'])
        self.size = int(subch_data['size'])
        self.protection = int(subch_data['protection'])
        self.bit_rate = self.size * 8/6

        # display info to selected sub-channel
        # service info
        self.label_service.setText(service_label['label'].strip())
        self.label_bit_rate.setText(str(subch_data['size']*8/6) + " kbps")
        # service component (=sub-channel) info
        self.label_primary.setText(("primary" if service_data['primary'] == True else "secondary"))
        self.label_dabplus.setText(("DAB+" if service_data['DAB+'] == True else "DAB"))
        # status Bar
        self.statusBar.showMessage("Play/Record the selected Service Component.")

    def snr_update(self):
        print "update snr"
        # display snr in progress bar if an instance of usrp_dab_rx is existing
        if hasattr(self, 'my_receiver'):
            SNR = self.my_receiver.get_snr()
            if SNR > 10:
                self.setStyleSheet("""QProgressBar::chunk { background: "green"; }""")
                if SNR > 20:
                    SNR = 20
            elif 5 < SNR <= 10:
                self.setStyleSheet("""QProgressBar::chunk { background: "yellow"; }""")
            else:
                self.setStyleSheet("""QProgressBar::chunk { background: "red"; }""")
                if SNR < -20 or math.isnan(SNR):
                    SNR = -20
            self.bar_snr.setValue(SNR)
            self.lcd_snr.display(SNR)
            self.snr_timer.start(500)
        else:
            self.bar_snr.setValue(-20)
            self.label_snr.setText("SNR: no reception")
            self.snr_timer.start(500)

    def play_audio(self):
        if not self.slider_volume.isEnabled():
            # play button pressed
            self.btn_play.setText("Mute")
            self.btn_stop.setEnabled(True)
            self.slider_volume.setEnabled(True)
            self.btn_update_info.setEnabled(True)
            self.slider_volume.setValue(self.volume)
            self.set_volume()
            # if selected sub-channel is not the current sub-channel we have to reconfigure the receiver
            if self.subch is not self.table_mci.currentRow():
                self.subch = self.table_mci.currentRow()
                dev_mode_opened = False
                if self.dev_mode_active:
                    dev_mode_opened = True
                self.dev_mode_close()
                self.snr_timer.stop()
                self.my_receiver.stop()
                self.my_receiver = usrp_dab_rx.usrp_dab_rx(self.frequency, self.bit_rate, self.address, self.size,
                                                           self.protection,
                                                           self.src_is_USRP, self.file_path, self.recorder)
                self.my_receiver.start()
                self.dev_mode_init()
                if dev_mode_opened:
                    self.dev_mode_open()
                self.snr_timer.start(5000)
        else:
            # mute button pressed
            self.btn_play.setText("Play")
            self.volume = self.slider_volume.value()
            self.slider_volume.setValue(0)
            self.slider_volume.setEnabled(False)
            self.set_volume()
            self.need_new_init = False
        # start timer for snr update again
        self.snr_timer.start(500)
        # update dev mode

    def stop_reception(self):
        # close dev mode
        self.btn_dev_mode_close.hide()
        self.btn_dev_mode_open.show()
        self.btn_dev_mode_open.setEnabled(False)
        # stop flowgraph
        self.my_receiver.stop()
        # enable/disable buttons
        self.btn_stop.setEnabled(False)
        self.btn_play.setText("Play")
        self.btn_play.setEnabled(True)
        self.slider_volume.setEnabled(False)
        self.btn_update_info.setEnabled(False)
        self.btn_record.setEnabled(True)
        self.recorder = False
        # stop snr updates because no flowgraph is running to measure snr
        self.snr_timer.stop()

    def record_audio(self):
        # enable/disable buttons
        self.btn_record.setEnabled(False)
        self.btn_play.setEnabled(False)
        self.btn_stop.setEnabled(True)
        self.recorder = True
        # start flowgraph
        self.my_receiver = usrp_dab_rx.usrp_dab_rx(self.frequency, self.bit_rate, self.address, self.size,
                                                   self.protection,
                                                   self.src_is_USRP, self.file_path, self.recorder)
        self.my_receiver.start()

    def set_volume(self):
        # map volume from [0:100] to [0:1]
        self.my_receiver.set_volume(float(self.slider_volume.value()) / 100)

    def get_ensemble_info(self):
        # load string (json) with ensemble info and convert it to dictionary
        # string structure example: "{\"SWR_BW_N\":{\"country_ID\":1}}"
        self.ensemble_info = json.loads(self.my_receiver.get_ensemble_info())
        json.dumps(self.ensemble_info)
        return self.ensemble_info

    def get_service_info(self):
        # load string (json) with MCI and convert it to array of dictionaries
        self.service_info = json.loads(self.my_receiver.get_service_info())
        # string structure example: "[{\"reference\":736,\"ID\":2,\"primary\":true},{\"reference\":736,\"ID\":3,\"primary\":false},{\"reference\":234,\"ID\":5,\"primary\":true}]"
        json.dumps(self.service_info)
        return self.service_info

    def get_service_labels(self):
        # load string (json) with service labels and convert it to array of dictionaries
        self.service_labels = json.loads(self.my_receiver.get_service_labels())
        # string structure example: "[{\"label\":\"SWR1_BW         \",\"reference\":736},{\"label\":\"SWR2            \",\"reference\":234}]"
        json.dumps(self.service_labels)
        return self.service_labels

    def get_subch_info(self):
        # load string (json) with sub-channel info and convert it to array of dictionaries
        self.subch_info = json.loads(self.my_receiver.get_subch_info())
        # string structure example: "[{\"ID\":2, \"address\":54, \"protect\":2,\"size\":84},{\"ID\":3, \"address\":54, \"protect\":2,\"size\":84}]"
        json.dumps(self.subch_info)
        return self.subch_info

    def get_programme_type(self):
        # load string (json) with service information (programme type) and convert it to array of dictionaries
        self.programme_type = json.loads(self.my_receiver.get_programme_type())
        # string structure example: "[{\"reference\":736, \"programme_type\":13},{\"reference\":234, \"programme_type\":0}]"
        json.dumps(self.programme_type)
        return self.programme_type

    def get_sample_rate(self):
        # TODO: set rational resampler in flowgraoph with sample rate
        return self.my_receiver.get_sample_rate()

    ################################
    # Developer Mode
    ################################

    def dev_mode_init(self):
        # FFT plot
        self.fft_plot = sip.wrapinstance(self.my_receiver.fft_plot.pyqwidget(), QtGui.QWidget)
        self.vertical_layout_dev_mode_right.addWidget(self.fft_plot)
        self.fft_plot.hide()
        # Waterfall plot
        self.waterfall_plot = sip.wrapinstance(self.my_receiver.waterfall_plot.pyqwidget(), QtGui.QWidget)
        self.vertical_layout_dev_mode_right.addWidget(self.waterfall_plot)
        self.waterfall_plot.hide()
        # Time plot
        # self.time_plot = sip.wrapinstance(self.my_receiver.time_plot.pyqwidget(), QtGui.QWidget)
        # self.vertical_layout_dev_mode_right.addWidget(self.time_plot)
        #self.time_plot.hide()
        # constellation plot
        self.constellation = sip.wrapinstance(self.my_receiver.constellation_plot.pyqwidget(), QtGui.QWidget)
        self.vertical_layout_dev_mode_right.addWidget(self.constellation)
        self.constellation.hide()
        # if dev mode is initialized, we can enable the dev mode open button
        self.btn_dev_mode_open.setEnabled(True)

    def dev_mode_open(self):
        self.dev_mode_active = True
        # hide open button and show close button
        self.btn_dev_mode_open.hide()
        self.btn_dev_mode_close.show()
        # show widgets
        self.fft_plot.show()
        self.waterfall_plot.show()
        self.constellation.show()
        self.label_firecode.show()
        self.label_firecode.setText("")
        self.content_count = 0

    def dev_mode_close(self):
        self.dev_mode_active = False
        # hide close button and show open button
        self.btn_dev_mode_close.hide()
        self.btn_dev_mode_open.show()
        # hide widgets
        self.fft_plot.hide()
        self.waterfall_plot.hide()
        self.constellation.hide()
        self.label_firecode.hide()

    def update_firecode(self):
        if self.dev_mode_active:
            if self.content_count >=120:
                self.label_firecode.setText("")
                self.content_count = 0
            if not self.my_receiver.get_firecode_passed():
                self.label_firecode.setText(self.label_firecode.text() + "<font color=\"red\">X </font>")
            else:
                errors = self.my_receiver.get_corrected_errors()
                self.label_firecode.setText(self.label_firecode.text() +("<font color=\"" + ("green" if (errors < 10) else "orange") + "\">" + str(errors) +" </font>"))
        self.label_firecode.setWordWrap(True)
        self.content_count += 1

    ################################
    # Transmitter functions
    ################################

    def t_set_sink_USRP(self):
        # enable/disable buttons
        self.t_btn_file_path.setEnabled(False)
        self.t_label_sink.setEnabled(False)
        self.t_spinbox_frequency.setEnabled(True)
        self.t_label_frequency.setEnabled(True)

    def t_set_sink_File(self):
        # enable/disable buttons
        self.t_btn_file_path.setEnabled(True)
        self.t_label_sink.setEnabled(True)
        self.t_spinbox_frequency.setEnabled(False)
        self.t_label_frequency.setEnabled(False)

    def t_change_num_subch(self):
        # get number of sub-channels from spin box
        num_subch = self.t_spin_num_subch.value()
        # update info text under the component fill in forms
        if num_subch is 7:
            self.t_label_increase_num_subch_info.setText("7 is the maximum number of components")
        else:
            self.t_label_increase_num_subch_info.setText("increase \"Number of channels\" for more components")
        # enable num_subch fill in forms for sub-channels
        if 0 <= num_subch <= 7:
            for n in range(0, 7):
                if n < num_subch:
                    self.components[n]["enabled"] = True
                else:
                    self.components[n]["enabled"] = False
        # display changes
        self.t_update_service_components()
        self.t_spin_listen_to_component.setMaximum(num_subch)

    def t_update_service_components(self):
        # display/hide components after the info in components (dict)
        for component in self.components:
            if component["enabled"] is False:
                component["label"].hide()
                component["data_rate_label"].hide()
                component["data_rate"].hide()
                component["protection_label"].hide()
                component["protection"].hide()
                component["src_label"].hide()
                component["src_path_disp"].hide()
                component["src_btn"].hide()
            else:
                component["label"].show()
                component["data_rate_label"].show()
                component["data_rate"].show()
                component["protection_label"].show()
                component["protection"].show()
                component["src_label"].show()
                component["src_path_disp"].show()
                component["src_btn"].show()

    def t_init_transmitter(self):
        self.statusBar.showMessage("initializing transmitter...")
        # boolean is set to True if info is missing to init the transmitter
        arguments_incomplete = False
        # produce array for protection and data_rate and src_paths
        protection_array = [None] * self.t_spin_num_subch.value()
        data_rate_n_array = [None] * self.t_spin_num_subch.value()
        audio_paths = ["None"] * self.t_spin_num_subch.value()
        for i in range(0, self.t_spin_num_subch.value()):
            protection_array[i] = self.components[i]["protection"].value()
            data_rate_n_array[i] = self.components[i]["data_rate"].value()/8
            if self.components[i]["src_path"] is "None":
                # highlight the path which is not selected
                self.components[i]["src_path_disp"].setStyleSheet('color: red')
                arguments_incomplete = True
                print "path " + str(i+1) + " not selected"
            else:
                audio_paths[i] = self.components[i]["src_path"]

        # check if File path for sink is chosen if option enabled
        if self.t_rbtn_File.isChecked() and (str(self.t_label_sink.text()) == "select path"):
            self.t_label_sink.setStyleSheet('color: red')
            arguments_incomplete = True

        if arguments_incomplete is False:
            # init transmitter
            self.my_transmitter = usrp_dab_tx.usrp_dab_tx(self.t_spinbox_frequency.value(),
                                              self.t_spin_num_subch.value(),
                                              str(self.t_edit_ensemble_label.text()),
                                              str(self.t_edit_service_label.text()),
                                              self.t_combo_language.currentIndex(),
                                              protection_array, data_rate_n_array,
                                              audio_paths,
                                              self.t_spin_listen_to_component.value(),
                                              self.t_rbtn_USRP.isChecked(),
                                              str(self.t_label_sink.text())+ "/" +str(self.t_edit_file_name.text()))
            # enable play button
            self.t_btn_play.setEnabled(True)
            self.t_label_status.setText("ready to transmit")

    def t_run_transmitter(self):
        self.t_btn_stop.setEnabled(True)
        self.t_slider_volume.setEnabled(True)
        self.t_label_status.setText("transmitting..")
        self.statusBar.showMessage("transmitting..")
        self.my_transmitter.start()

    def t_set_volume(self):
        self.my_transmitter.set_volume(float(self.t_slider_volume.value())/100)

    def t_stop_transmitter(self):
        # stop flowgraph
        self.my_transmitter.stop()
        self.t_btn_stop.setEnabled(False)
        self.t_slider_volume.setEnabled(False)
        self.t_label_status.setText("not running")
        self.statusBar.showMessage("not running")

    def t_set_file_path(self):
        path = QtGui.QFileDialog.getExistingDirectory(self, "Pick a folder for your file sink")
        if path:  # if user didn't pick a directory don't continue
            # display path in path label
            self.t_label_sink.setText(path)
            self.t_label_sink.setStyleSheet('color: black')

    def t_set_subch_path1(self):
        path = QtGui.QFileDialog.getOpenFileName(self, "Pick a .wav file as audio source")
        if path:  # if user didn't pick a directory don't continue
            # set new path in dict components
            self.components[0]["src_path"] = str(path)
            # display path in path label
            self.components[0]["src_path_disp"].setText(path)
            self.components[0]["src_path_disp"].setStyleSheet('color: black')

    def t_set_subch_path2(self):
        path = QtGui.QFileDialog.getOpenFileName(self, "Pick a .wav file as audio source")
        if path:  # if user didn't pick a directory don't continue
            # set new path in dict components
            self.components[1]["src_path"] = str(path)
            # display path in path label
            self.components[1]["src_path_disp"].setText(path)
            self.components[1]["src_path_disp"].setStyleSheet('color: black')

    def t_set_subch_path3(self):
        path = QtGui.QFileDialog.getOpenFileName(self, "Pick a .wav file as audio source")
        if path:  # if user didn't pick a directory don't continue
            # set new path in dict components
            self.components[2]["src_path"] = str(path)
            # display path in path label
            self.components[2]["src_path_disp"].setText(path)
            self.components[2]["src_path_disp"].setStyleSheet('color: black')

    def t_set_subch_path4(self):
        path = QtGui.QFileDialog.getOpenFileName(self, "Pick a .wav file as audio source")
        if path:  # if user didn't pick a directory don't continue
            # set new path in dict components
            self.components[3]["src_path"] = str(path)
            # display path in path label
            self.components[3]["src_path_disp"].setText(path)
            self.components[3]["src_path_disp"].setStyleSheet('color: black')

    def t_set_subch_path5(self):
        path = QtGui.QFileDialog.getOpenFileName(self, "Pick a .wav file as audio source")
        if path:  # if user didn't pick a directory don't continue
            # set new path in dict components
            self.components[4]["src_path"] = str(path)
            # display path in path label
            self.components[4]["src_path_disp"].setText(path)
            self.components[4]["src_path_disp"].setStyleSheet('color: black')

    def t_set_subch_path6(self):
        path = QtGui.QFileDialog.getOpenFileName(self, "Pick a .wav file as audio source")
        if path:  # if user didn't pick a directory don't continue
            # set new path in dict components
            self.components[5]["src_path"] = str(path)
            # display path in path label
            self.components[5]["src_path_disp"].setText(path)
            self.components[5]["src_path_disp"].setStyleSheet('color: black')

    def t_set_subch_path7(self):
        path = QtGui.QFileDialog.getOpenFileName(self, "Pick a .wav file as audio source")
        if path:  # if user didn't pick a directory don't continue
            # set new path in dict components
            self.components[6]["src_path"] = str(path)
            # display path in path label
            self.components[6]["src_path_disp"].setText(path)
        self.components[7]["src_path_disp"].setStyleSheet('color: black')


class lookup_tables:
    languages = [
        "unknown language",
        "Albanian",
        "Breton",
        "Catalan",
        "Croatian",
        "Welsh",
        "Czech",
        "Danish",
        "German",
        "English",
        "Spanish",
        "Esperanto",
        "Estonian",
        "Basque",
        "Faroese",
        "French",
        "Frisian",
        "Irish",
        "Gaelic",
        "Galician",
        "Icelandic",
        "Italian",
        "Lappish",
        "Latin",
        "Latvian",
        "Luxembourgian",
        "Lithuanian",
        "Hungarian",
        "Maltese",
        "Dutch",
        "Norwegian",
        "Occitan",
        "Polish",
        "Postuguese",
        "Romanian",
        "Romansh",
        "Serbian",
        "Slovak",
        "Slovene",
        "Finnish",
        "Swedish",
        "Tuskish",
        "Flemish",
        "Walloon"
    ]
    programme_types = [
        "No programme type",
        "News",
        "Current Affairs",
        "Information",
        "Sport",
        "Education",
        "Drama",
        "Culture",
        "Science",
        "Varied",
        "Pop Music",
        "Rock Music",
        "Easy Listening Music",
        "Light Classical",
        "Serious Classical",
        "Other Music",
        "Weather/meteorology",
        "Finance/Business",
        "Children's programmes",
        "Social Affairs",
        "Religion",
        "Phone In",
        "Travel",
        "Leisure",
        "Jazz Music",
        "Country Music",
        "National Music",
        "Oldies Music",
        "Folk Music",
        "Documentary",
        "None",
        "None"
    ]
    country_ID_ECC_E0 = [
        "None",
        "Germany",
        "Algeria",
        "Andorra",
        "Israel",
        "Italy",
        "Belgium",
        "Russian Federation",
        "Azores (Portugal)",
        "Albania",
        "Austria",
        "Hungary",
        "Malta",
        "Germany",
        "Canaries (Spain)",
        "Egypt"
    ]
    protection_EEP_set_A = [
        "1-A",
        "2-A",
        "3-A",
        "4-A"
    ]

def main():
    app = QtGui.QApplication(sys.argv)
    form = DABstep()
    form.show()
    app.exec_()


if __name__ == '__main__':
    main()
