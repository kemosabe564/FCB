import os
os.environ['PYGAME_HIDE_SUPPORT_PROMPT'] = "hide"
import pygame
import argparse

from communication.serial import Serial
from communication.command import Command, CommandType, SerialCommandDecoder
from communication.cli import CLI, CLIAction
from communication.crc8 import crc8

ser = None
cli = None


def new_cmd_handler(data):
    # cli.to_cli(data)
    pass


def new_action_handler(action, data=None):
    if action == CLIAction.SendCommand:
        ser.send_command(data)

    if action == CLIAction.Exit:
        ser.stop()
        cli.stop()

    if action == CLIAction.SetProtocol:
        ser.set_protocol(data)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument('--port', type=str, default='/dev/ttyUSB0', help='COM port of the drone')
    parser.add_argument('--baud', type=int, default=115200, help='Baudrate for communication')

    args = parser.parse_args()

    ser = Serial(port=args.port, baud=args.baud, command_handler=new_cmd_handler)
    cli = CLI(action_handler=new_action_handler)

    cli.join()
    ser.join()
