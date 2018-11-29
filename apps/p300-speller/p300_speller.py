import os
import time
import pandas as pd
import numpy as np
import random
try:
    # for Python2
    import Tkinter as tk
    from Tkinter import *
except ImportError:
    # for Python3
    import tkinter as tk
    from tkinter import *
import argparse
import yaml
import pickle
import logging

from brainflow import *
import classifier


class StartScreen (tk.Frame):
    """Starting screen for the application"""
    def __init__ (self, master, next_screen):
        tk.Frame.__init__ (self, master)
        training_instructions = ('1) A character will be highlighted at the\n'
                        'beginning of each trial\n\n'
                        '2) Fixate on the character\n\n'
                        '3) Rows and columns will begin to flash\n\n'
                        '3) Continue to fixate on the character until\n'
                        'another character is highlighted\n')

        live_instructions = ('1) Fixate on the character you wish to select\n\n'
                    '2) A character will be predicted and types after\n'
                    'a set amount of rounds\n')

        self.title_text = tk.Label (self, text = "Brainflow P300 Speller", font = ('Arial', 24))
        self.title_text.grid ()

        self.directions_label = tk.Label (self, text = 'Directions:', font = ('Arial', 18))
        self.directions_label.grid (sticky = tk.W)

        self.training_label = tk.Label(self, text = 'Training:', font = ('Arial', 16))
        self.training_label.grid (sticky = tk.W)

        self.training_text = tk.Label (self, text = training_instructions, font = ('Arial', 14), justify = LEFT)
        self.training_text.grid (sticky = tk.W)

        self.live_label = tk.Label (self, text = 'Live Spelling:', font = ('Arial', 16))
        self.live_label.grid (sticky = tk.W)

        self.live_text = tk.Label (self, text = live_instructions, font = ('Arial', 14), justify = LEFT)
        self.live_text.grid (sticky = tk.W)

        self.start_training_button = tk.Button (self, command = self.start_training, text = 'Train', font = ('Arial', 24, 'bold'), height = 4, width = 24)
        self.start_training_button.grid (pady = 3, sticky = tk.W + tk.E)

        self.start_live_button = tk.Button (self, command = self.start_live, text = 'Live', font = ('Arial', 24, 'bold'), height = 4, width = 24)
        self.start_live_button.grid (pady = 3, sticky = tk.W + tk.E)

        self.next_screen = next_screen

    def display_screen (self):
        """Adds this screen to the window"""
        self.place (relx = 0.5, rely = 0.5, anchor = CENTER)

    def remove_screen (self):
        """Removes this screen from the window"""
        self.place_forget ()

    def start_training (self):
        self.next_screen.set_training_mode (True)
        self.__start_speller ()

    def start_live (self):
        self.next_screen.set_training_mode (False)
        self.__start_speller ()

    def __start_speller (self):
        """Removes this frame and displays the grid of characters"""
        self.next_screen.display_screen ()
        self.next_screen.update ()
        self.remove_screen ()


class P300GUI (tk.Frame):
    """The main screen of the application that displays the character grid and spelling buffer"""
    def __init__ (self, master, settings):
        tk.Frame.__init__ (self, master)
        # df to store events
        self.event_data = pd.DataFrame (columns = ['event_start_time', 'orientation', 'highlighted', 'trial_row', 'trial_col'])
        self.is_streaming = False
        self.num_cols = settings['general']['num_cols']
        self.settings = settings
        self.grid_width = settings['window_settings']['grid_width']
        self.is_training = False
        self.highlight_time = settings['presentation']['highlight_time']
        self.intermediate_time = settings['presentation']['intermediate_time']
        self.col_width = self.grid_width / self.num_cols
        self.selection_rect = self.make_rectangle ()
        self.canvas = tk.Canvas (self)
        self.spelled_text = tk.StringVar ()
        self.spelled_text.set ('')
        self.text_buffer = tk.Entry (self, font = ('Arial', 24, 'bold'), cursor = 'arrow',
                                    insertbackground = '#ffffff', textvariable = self.spelled_text)
        self.trial_row = -1
        self.trial_col = -1
        self.trial_count = 0
        self.sequence_count = 0
        self.trial_in_progress = False
        self.char_highlighted = False

        self.char_select_rect = SelectionRectangle (
                                                    self.settings,
                                                    x = self.col_width * self.trial_col, y = self.col_width * self.trial_row,
                                                    width = self.col_width, length = self.col_width,
                                                    max_x = self.grid_width, max_y = self.grid_width,
                                                    color = self.settings['presentation']['char_select_color']
                                                    )
        self.create_widgets ()
        if settings['general']['board'] == 'Cython':
            self.board_id = CYTHON.board_id
            self.board = BoardShim (self.board_id, settings['general']['port'])
            self.board.prepare_session ()
        else:
            raise ValueError ('unsupported board type')

    def set_training_mode (self, is_training):
        self.is_training = is_training
        if not self.is_training:
            with open (os.path.join (os.path.dirname (os.path.realpath (__file__)), 'data','scaler.pickle'), 'rb') as f:
                self.scaler = pickle.load (f)
            with open (os.path.join (os.path.dirname (os.path.realpath (__file__)), 'data','pca.pickle'), 'rb') as f:
                self.pca = pickle.load (f)
            with open (os.path.join (os.path.dirname (os.path.realpath (__file__)), 'data','classifier.pickle'), 'rb') as f:
                self.classifier = pickle.load (f)

            self.current_live_count = 0
            self.cols_weights = numpy.zeros (self.settings['general']['num_cols']).astype (numpy.float64)
            self.rows_weights = numpy.zeros (self.settings['general']['num_cols']).astype (numpy.float64)
            self.cols_predictions = numpy.zeros (self.settings['general']['num_cols']).astype (numpy.int64)
            self.rows_predictions = numpy.zeros (self.settings['general']['num_cols']).astype (numpy.int64)

    def display_screen (self):
        """Adds this screen to the window"""
        self.place (relx = 0.5, rely = 0.5, anchor = CENTER)

    def remove_screen (self):
        """Removes this screen from the window"""
        self.place_forget ()

    def update (self):
        """Updates the gui based on the mode the application is in"""
        if not self.is_streaming:
            self.board.start_stream (7200 * self.board.fs_hz)
            self.is_streaming = True
        # Moves the selection rect off-screen
        self.selection_rect.move_to_col (-2)
        if self.is_training:
            self.training_update ()
        else:
            self.live_update ()

    def training_update (self):
        """Updates the gui while in training mode"""
        # Highlight the character when we are currently not in the middle of a trial
        if not self.trial_in_progress:
            self.get_training_character ()
            # Move the char highlight rect behind the character
            self.char_select_rect.move_to_col (self.trial_col, reset_top = False)
            self.char_select_rect.move_to_row (self.trial_row, reset_left = False)
            # highlight the character
            self.char_highlighted = True
            self.trial_in_progress = True
            self.draw ()
            self.spelled_text.set ('Look at: %s' % str (self.get_character (self.trial_row, self.trial_col)))
            # Wait
            self.master.after (self.settings['training_params']['wait_timeout'], self.update)

        elif self.trial_in_progress:
            # Turn off the highlighting of the character
            if self.char_highlighted:
                self.char_highlighted = False
                self.draw ()
            # Proceed updating like normal
            if self.selection_rect.visible:
                # Update the position of the rectangle
                self.selection_rect.update ()
                self.record_event ()
                # Rectangle is set to visible, draw the canvas
                self.draw ()
                # Set it visibility for when this function is called again
                self.selection_rect.visible = False
                # Allow the rectangle to remain visible for a set time
                self.master.after (self.highlight_time, self.update)
            else:
                # Rectangle is set to invisible, update the canvas
                self.draw ()
                # Set visibility to visible for next update call
                self.selection_rect.visible = True

                if self.selection_rect.end_of_sequence ():
                    self.sequence_count = self.sequence_count + 1
                    if self.sequence_count >= self.settings['training_params']['seq_per_trial']:
                        self.trial_count = self.trial_count + 1
                        self.sequence_count = 0
                        self.trial_in_progress = False
                        if self.trial_count >= self.settings['training_params']['num_trials']:
                            self.write_and_exit ()
                        else:
                            self.master.after (self.settings['general']['epoch_length'] + self.intermediate_time, self.update)
                    else:
                        self.master.after (self.settings['general']['epoch_length'] + self.intermediate_time, self.update)
                else:
                    # Keep the rect invisible for a set amount of time
                    self.master.after (self.intermediate_time, self.update)


    def live_update (self):
        """Updates the position and visibility of the selection rectangle"""
        if self.selection_rect.visible:
            self.selection_rect.update ()
            self.record_event ()
            self.draw ()
            self.selection_rect.visible = False
            # Allow the rectangle to remain visible for a set time
            self.master.after (self.highlight_time, self.update)
        else:
            self.draw ()
            # Set visibility to visible for next update call
            self.selection_rect.visible = True
            if self.selection_rect.end_of_sequence ():
                self.sequence_count = self.sequence_count + 1
                if self.sequence_count >= self.settings['live_params']['seq_per_trial']:
                    self.master.after (self.settings['general']['epoch_length'] + self.intermediate_time, self.update)
                    time.sleep (self.settings['general']['epoch_length'] / 1000.0)

                    # get predicted character
                    predicted_row, predicted_col = self.get_predicted ()
                    # free old events
                    self.event_data = pd.DataFrame (columns = ['event_start_time', 'orientation', 'highlighted', 'trial_row', 'trial_col'])
                    if predicted_row is not None and predicted_col is not None:
                        predicted_char = self.get_character (predicted_row, predicted_col)
                        self.add_text (predicted_char)
                        self.free_live_variables ()
                    else:
                        logging.debug ('trying to expand data')

                    self.sequence_count = 0
                else:
                    self.master.after (self.settings['general']['epoch_length'] + self.intermediate_time, self.update)
            else:
                # Keep the rect invisible for a set amount of time
                self.master.after (self.intermediate_time, self.update)

    def free_live_variables (self):
        # free current weights
        self.current_live_count = 0
        self.cols_weights = numpy.zeros (self.settings['general']['num_cols']).astype (numpy.float64)
        self.rows_weights = numpy.zeros (self.settings['general']['num_cols']).astype (numpy.float64)
        self.cols_predictions = numpy.zeros (self.settings['general']['num_cols']).astype (numpy.int64)
        self.rows_predictions = numpy.zeros (self.settings['general']['num_cols']).astype (numpy.int64)

    def get_character (self, row, col):
        """Returns the character from the grid at the given row and column"""
        cell_num = (row * self.num_cols) + col
        if cell_num <= 25:
            return chr (65 + cell_num)
        else:
            return str (cell_num - 26)

    def draw (self):
        """Redraws the canvas"""
        self.canvas.delete ('all')
        if self.char_highlighted:
            self.selection_rect.x = -10000
            self.selection_rect.x = -10000
            self.char_select_rect.draw (self.canvas)
        else:
            self.selection_rect.draw (self.canvas)
        self.draw_characters ()

    def make_rectangle (self, orientation = 'vertical'):
        """Returns a new selection rectangle for this GUI"""
        if orientation == 'vertical':
            return SelectionRectangle (
                                        self.settings,
                                        x = 0, y = 0,
                                        width = self.col_width, length = self.grid_width,
                                        color = self.settings['presentation']['rect_color'],
                                        max_x = self.grid_width, max_y = self.grid_width
                                      )
        else:
            return SelectionRectangle (
                                        self.settings,
                                        x = 0, y = 0,
                                        width = self.grid_width,
                                        length = self.col_width,
                                        color = self.settings['presentation']['rect_color'],
                                        max_x = self.grid_width, max_y = self.grid_width
                                      )

    def draw_characters (self):
        """Draws 36 characters [a-z] and [0-9] in a 6x6 grid"""
        row_height = int (self.canvas['height']) / self.num_cols

        # Draw the characters to the canvas
        ascii_letter_offset = 65
        ascii_number_offset = 48
        ascii_offset = ascii_letter_offset
        current_offset = 0

        for row in range (self.num_cols):
            for col in range (self.num_cols):
                # Case that we have gone through all characters
                if current_offset == 26 and ascii_offset == ascii_letter_offset:
                    ascii_offset = ascii_number_offset
                    current_offset = 0
                # Case that we have gone though all of the numbers
                elif current_offset == 10 and ascii_offset == ascii_number_offset:
                    break
                # Get the current cell character
                cell_char = chr (ascii_offset + current_offset)
                current_offset =  current_offset + 1
                canvas_id = self.canvas.create_text ((self.col_width * col) + (self.col_width / 2.5),
                                                    (row_height * row) + (row_height / 3),
                                                    font = ('Arial', (self.col_width / 4), 'bold'),
                                                    anchor = 'nw')

                # Determine if this character is printed white or black
                if self.selection_rect != None:
                    if ((self.selection_rect.is_vertical () and col == self.selection_rect.get_index ()
                         or not self.selection_rect.is_vertical () and row == self.selection_rect.get_index ())
                         and self.selection_rect.visible):
                        self.canvas.itemconfig (canvas_id, text = cell_char, fill = self.settings['presentation']['highlight_char_color'])
                    else:
                        self.canvas.itemconfig (canvas_id, text = cell_char, fill = self.settings['presentation']['default_char_color'])

    def add_space (self):
        """Adds a space '_' to the spelled text buffer"""
        self.spelled_text.set (self.spelled_text.get () + "_")
        self.text_buffer.icursor (len (self.spelled_text.get ()))

    def delete_last (self):
        """Deletes the last character in the spelled text buffer"""
        if len (self.spelled_text.get ()) > 0:
            self.spelled_text.set (self.spelled_text.get ()[:-1])
            self.text_buffer.icursor (len (self.spelled_text.get ()))

    def add_text (self, text):
        """Appends some given text to the sppelled text buffer"""
        self.spelled_text.set (self.spelled_text.get () + text)
        self.text_buffer.icursor (len (self.spelled_text.get ()))

    def create_widgets(self):
        """Populates the gui with all the necessary components"""
        self.master['bg'] = '#001c33'
        self['bg'] = '#001c33'
        # Displays the current text being typed
        self.text_buffer.grid (row = 0, pady = 20, sticky = tk.W + tk.E)
        self.text_buffer['fg'] = '#ffffff'
        self.text_buffer['bg'] = '#000000'
        # Canvas for drawing the grid of characters and the rectangle
        self.canvas['width'] = self.grid_width
        self.canvas['height'] = self.canvas['width']
        self.canvas['bg'] = self.settings['presentation']['grid_bg_color']
        self.canvas.grid (row = 2, sticky = tk.W + tk.E)
        # Frame to hold all buttons at the bttom of the gui
        self.bottom_button_pane = tk.Frame (self)
        self.bottom_button_pane.grid (pady = 10)
        # Button to delete the previous character
        self.back_space_button = tk.Button (self.bottom_button_pane, text = 'delete', command = self.delete_last, height = 1, width = 6)
        self.back_space_button.grid (row = 0,column = 0)
        # Button for adding a space character to the text_buffer
        self.space_button = tk.Button (self.bottom_button_pane, text = 'space', command = self.add_space, height = 1, width = 12)
        self.space_button.grid (row = 0,column = 1)
        # Button for exiting the application
        self.exit_button = tk.Button (self.bottom_button_pane, text = 'exit', command = self.write_and_exit, height = 1, width = 6)
        self.exit_button.grid (row = 0,column = 3)

    def get_training_character (self):
        """Set new training character"""
        self.trial_col = (self.trial_col + 1) % self.num_cols
        if self.trial_col == 0:
            self.trial_row = (self.trial_row + 1) % self.num_cols

    def record_event (self):
        """Sends epoch event codes and times to the main process"""
        index = self.selection_rect.get_index ()
        if self.selection_rect.is_vertical ():
            orientation = 'col'
        else:
            orientation = 'row'
        if self.is_training:
            self.event_data = self.event_data.append ({'event_start_time' : time.time (), 'orientation' : orientation, 'highlighted':index,
                                                        'trial_row': self.trial_row, 'trial_col' : self.trial_col}, ignore_index = True)
        else:
            self.event_data = self.event_data.append ({'event_start_time' : time.time (), 'orientation' : orientation, 'highlighted':index}, ignore_index = True)

    def write_and_exit (self):
        if self.is_training:
            event_file = os.path.join (os.path.dirname (os.path.abspath (__file__)), 'data', 'events.csv')
            if os.path.isfile (event_file):
                self.event_data.to_csv (event_file, mode = 'a', header = False, index = False)
            else:
                self.event_data.to_csv (event_file, index = False)

            data = self.board.get_board_data ()
            data_handler = DataHandler (self.board_id, numpy_data = data)
            data_handler.save_csv (os.path.join (os.path.dirname (os.path.abspath (__file__)), 'data', 'eeg.csv'))
        self.master.quit ()

    def get_predicted (self):
        """perform prediction"""
        self.current_live_count = self.current_live_count + 1

        eeg_data = self.board.get_current_board_data (int (CYTHON.fs_hz * (self.settings['general']['epoch_length'] * (self.settings['live_params']['seq_per_trial'] + 2)) / 1000.0 ))
        data_handler = DataHandler (self.board_id, numpy_data = eeg_data)
        eeg_data = data_handler.get_data ()
        eeg_data.index.name = 'index'
        self.event_data.index.name = 'index'

        data_x, _ = classifier.prepare_data (eeg_data, self.event_data, self.settings, False)
        if data_x.shape[0] != self.settings['live_params']['seq_per_trial'] * self.settings['general']['num_cols'] * 2:
            logging.error ('Incorrect data shape')
        decisions = classifier.get_decison (data_x, self.scaler, self.pca, self.classifier)

        for i, decision in enumerate (decisions):
            event = self.event_data.iloc[i,:]
            if event['orientation'] == 'col':
                if decision > 0:
                    self.cols_predictions[event['highlighted']] = self.cols_predictions[event['highlighted']] + 1
                self.cols_weights[event['highlighted']] = self.cols_weights[event['highlighted']] + decision
            else:
                if decision > 0:
                    self.rows_predictions[event['highlighted']] = self.rows_predictions[event['highlighted']] + 1
                self.rows_weights[event['highlighted']] = self.rows_weights[event['highlighted']] + decision

        best_col_id = None
        best_row_id = None

        max_col_predictions = 0
        max_col_decision = 0
        max_row_predictions = 0
        max_row_decision = 0
        for i in range (self.settings['general']['num_cols']):
            if self.cols_predictions[i] > max_col_predictions:
                best_col_id = i
                max_col_predictions = self.cols_predictions[i]
                max_col_decision = self.cols_weights[i]
            elif self.cols_predictions[i] == max_col_predictions and max_col_decision < self.cols_weights[i]:
                best_col_id = i
                max_col_predictions = self.cols_predictions[i]
                max_col_decision = self.cols_weights[i]

            if self.rows_predictions[i] > max_row_predictions:
                best_row_id = i
                max_row_predictions = self.rows_predictions[i]
                max_row_decision = self.rows_weights[i]
            elif self.rows_predictions[i] == max_row_predictions and max_row_decision < self.rows_weights[i]:
                best_row_id = i
                max_row_predictions = self.rows_predictions[i]
                max_row_decision = self.rows_weights[i]


        second_col_score = sorted (self.cols_predictions.tolist ())[-2]
        second_row_score = sorted (self.rows_predictions.tolist ())[-2]
        if self.current_live_count < 3:
            if best_col_id is not None:
                val = self.cols_predictions[best_col_id]
                if ((float (val - second_col_score)) / val < self.settings['live_params']['stop_thresh']):
                    best_col_id = None
                if val < 2:
                    best_col_id = None

            if best_row_id is not None:
                val = self.rows_predictions[best_row_id]
                if ((float (val - second_row_score)) / val < self.settings['live_params']['stop_thresh']):
                    best_row_id = None
                if val < 2:
                    best_row_id = None

        logging.debug ('predicted cols %s' % ' '.join ([str (x) for x in self.cols_predictions]))
        logging.debug ('predicted rows %s' % ' '.join ([str (x) for x in self.rows_predictions]))
        logging.debug ('predicted col: %s predicted row:%s' % (str (best_col_id), str (best_row_id)))
        return best_row_id, best_col_id


class SelectionRectangle ():
    """Manages the rectangle that highlights the characters in the grid"""
    def __init__ (self, settings, x, y, length, width, max_x, max_y, color = '#ffffff'):
        self.settings = settings
        # x,y - top left position
        self.x = x
        self.y = y
        self.length = length
        self.width = width
        self.graphic_ref = None
        self.color = color
        self.max_x = max_x
        self.max_y = max_y
        self.remaining_rows = range (6)
        self.remaining_cols = range (6)
        self.visible = True

    def get_index (self):
        """Return the current row or column index of the rectangle"""
        if self.is_vertical ():
            return int (self.x / self.width)
        else:
            return int (self.y / self.length)

    def move_to_col (self, index, reset_top = True):
        """Moves and re-orients the rectangle to a column specified by an index"""
        # Reorient the rectangle to be vertical
        if not self.is_vertical():
            self.rotate90 ()
        # Set the rectangle to the proper position
        self.x = index * self.width
        if reset_top:
            self.y = 0

    def move_to_row (self, index, reset_left = True):
        """Moves and re-orients the rectangle to a row specified by an index"""
        # Reorient the rectangle to be horizontal
        if self.is_vertical ():
            self.rotate90 ()
        # Set the rectangel to the proper position
        self.y = index * self.length
        if reset_left:
            self.x = 0

    def rotate90 (self):
        """Rotates the rectangle 90 degrees"""
        temp = self.width
        self.width = self.length
        self.length = temp

    def move_vertical (self, distance):
        """Moves the rectangle by some distance in the y-direction"""
        self.y += distance

    def move_horizontal (self, distance):
        """Moves the rectangle by some distance in the x-direction"""
        self.x += distance

    def is_vertical (self):
        """Returns true if the rectangle is oriented vertically"""
        return self.length > self.width

    def refill_available_rcs (self):
        """Refills the lists of available rows and columns with index values"""
        self.remaining_rows = range (6)
        self.remaining_cols = range (6)

    def select_rand_row (self):
        """Selects a row from the available_rows"""
        rand_index = random.randint (0, len (self.remaining_rows) - 1)
        row = self.remaining_rows[rand_index]
        return row

    def select_rand_col (self):
        """Selects a random column from the available_cols"""
        rand_index = random.randint (0, len (self.remaining_cols) - 1)
        col = self.remaining_cols[rand_index]
        return col

    def end_of_sequence (self):
        """Returns true if there are no more available moves for the rect"""
        return len (self.remaining_cols) == 0 and len (self.remaining_rows) == 0

    def update (self):
        """Moves the recangle by one row or column and creates epoch"""
        # Move the rectangle by randomly selecting a row or column
        if self.settings['presentation']['random_highlight']:
            # The remaining columns and row lists need to be refilled
            if self.end_of_sequence ():
                self.refill_available_rcs ()

            # Freely choose between available rows and columns
            if len (self.remaining_cols) > 0 and len (self.remaining_rows) > 0:
                if random.random () > 0.5:
                    next_col = self.select_rand_col ()
                    self.move_to_col (next_col)
                    self.remaining_cols.remove (next_col)
                else:
                    next_row = self.select_rand_row ()
                    self.move_to_row (next_row)
                    self.remaining_rows.remove (next_row)

            elif len (self.remaining_cols) == 0:
                next_row = self.select_rand_row ()
                self.move_to_row (next_row)
                self.remaining_rows.remove (next_row)

            elif len (self.remaining_rows) == 0:
                next_col = self.select_rand_col ()
                self.move_to_col (next_col)
                self.remaining_cols.remove (next_col)

        # Move linearly through all the rows and columns
        else:
            if self.is_vertical ():
                self.move_horizontal (self.width)
                if self.x + self.width > self.max_x:
                    self.x = 0
                    self.rotate90 ()
            else:
                self.move_vertical (self.length)
                if self.y + self.length > self.max_y:
                    self.y = 0
                    self.rotate90 ()

    def draw (self, canvas):
        """Draws the rectange to a Tkinter canvas"""
        if self.visible:
            if self.graphic_ref != None:
                canvas.delete (self.graphic_ref)
            self.graphic_ref = canvas.create_rectangle (
                                                        self.x,
                                                        self.y,
                                                        self.x + self.width,
                                                        self.y + self.length,
                                                        fill = self.color
                                                        )


def main ():
    logging.basicConfig (level = logging.DEBUG)

    parser = argparse.ArgumentParser ()
    parser.add_argument ('--settings', type = str, help  = 'settings file', default = 'ui_settings.yml')
    args = parser.parse_args ()

    settings = yaml.load (open (args.settings))
    window_settings = settings['window_settings']

    root = tk.Tk ()
    root.title ('Brainflow P300 speller')
    root.protocol ('WM_DELETE_WINDOW', root.quit)
    root.geometry ('{}x{}'.format (window_settings['geometry_x'], window_settings['geometry_y']))
    root.resizable (width = False, height = False)
    speller_gui = P300GUI (root, settings)
    start_screen = StartScreen (root, speller_gui)
    start_screen.display_screen ()
    root.mainloop ()


if __name__ == "__main__":
    main ()
