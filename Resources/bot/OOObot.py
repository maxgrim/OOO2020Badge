#!/usr/bin/env python
# -*- coding: utf-8 -*-
# This program is dedicated to the public domain under the CC0 license.

"""
Simple Bot to reply to Telegram messages.

First, a few handler functions are defined. Then, those functions are passed to
the Dispatcher and registered at their respective places.
Then, the bot is started and runs until we press Ctrl-C on the command line.

Usage:
Basic Echobot example, repeats messages.
Press Ctrl-C on the command line or send a signal to the process to stop the
bot.
"""

import logging
import time
from telegram.ext import Updater, CommandHandler, MessageHandler, Filters

# Enable logging
logging.basicConfig(
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s', level=logging.INFO
)

logger = logging.getLogger(__name__)


# Define a few command handlers. These usually take the two arguments update and
# context. Error handlers also receive the raised TelegramError object in error.
def start(update, context):
    """Send a message when the command /start is issued."""
    update.message.reply_text('Hi! Glad you found me. Now I am not so lonely and have another friend. Are you ready to play?')


def help_command(update, context):
    """Send a message when the command /help is issued."""
    update.message.reply_text('Let me help you. \r\n /help print this help \r\n /safety prints safety instructions \r\n /play start the game\r\n /joingroup Join CTF tg group')

def play_command(update,context):
    """Send a message when the command /play is issued"""
    update.message.reply_text('Rkrt: Welcome on board. Let\'s see if you are worth the challenge. To find the invite code and land on planet hackazon you will need to solve this first. Ready for a ride?!')
    time.sleep(5)
    update.message.reply_text('Mx: During intergalactical travel, time does not matter. Any enemies could be listening in at any time. This is why the crew is sometimes forced to used coded languages to exchange messages between vessels. To decrypt messages every crew member can use the key on their hardware tokens.')
    time.sleep(10)
    update.message.reply_text('Jms: Mx we are getting a distress signal from vessel Vigenere. Do you copy?')
    time.sleep(3)
    update.message.reply_text('Mx: [gasps...]')
    time.sleep(1)
    update.message.reply_text('Mx: This one is for you rookie... See you on the other side.')
    update.message.reply_text('Kyjkda kghc tir Yeevobyj: BgXfsGofrCyrDouwfh\r\nUsfcfqg zb dywzv lcfy ij cqff hsnal jjoa:\r\nCKJ{en55td2my6jse8361a427p3xf319tf12}')


def safety_command(update,context):
    """Send a message when the command /safety is issued"""
    update.message.reply_text('Quick safety instructions about the badge: \r\nThis device was designed and assembled by us and has not been certified by any standards. This means that, despite our efforts, the device might cause unintended damage to whomever operates the device or your surroundings.\r\nUse the product with care and responsibly. We are not liable for any damages which can occur while operating/using the device. This was mainly designed to play a supervised CTF.\r\nThe device only draws 5v of power, so there is no risk of electrocution. However, do not use a (non standard) USB power outlet with a higher voltage. If device produces abnormal heat or \'magic smoke\': remove the power and do not turn on again. We will be happy to send you a replacement.\r\nIf a short circuit occurs while running on batteries, the batteries will get hot. So:\r\n')
    time.sleep(1)
    update.message.reply_text('DO NOT LEAVE THE DEVICE TURNED ON UNATTENDED')
    time.sleep(1)
    update.message.reply_text('The circuitry is fully exposed and metal pins are sticking out from the device. We strongly recommend not to leave children play with the device unattended.')
    time.sleep(1)
    update.message.reply_text('If you have any questions about safety you can reach out to the team by email or via the TG chat accessible via /joingroup')

def joingroup_command(update,context):
    """Send the group link to the user"""
    update.message.reply_text('Want to chat with other CTF players or ask questions to admins? Use the following channel:\r\nhttps://t.me/joinchat/CYsj-xwzlFqIbQPPeo04bw')
def echo(update, context):
    """Rick roll the user."""
    update.message.reply_text('Did you mean? https://www.youtube.com/watch?v=dQw4w9WgXcQ ?')



def main():
    """Start the bot."""
    # Create the Updater and pass it your bot's token.
    # Make sure to set use_context=True to use the new context based callbacks
    # Post version 12 this will no longer be necessary
    updater = Updater("<key>", use_context=True)

    # Get the dispatcher to register handlers
    dp = updater.dispatcher

    # on different commands - answer in Telegram
    dp.add_handler(CommandHandler("start", start))
    dp.add_handler(CommandHandler("help", help_command))
    dp.add_handler(CommandHandler("play", play_command))
    dp.add_handler(CommandHandler("safety", safety_command))
    dp.add_handler(CommandHandler("joingroup", joingroup_command))
    # on noncommand i.e message - echo the message on Telegram
    dp.add_handler(MessageHandler(Filters.text & ~Filters.command, echo))

    # Start the Bot
    updater.start_polling()

    # Run the bot until you press Ctrl-C or the process receives SIGINT,
    # SIGTERM or SIGABRT. This should be used most of the time, since
    # start_polling() is non-blocking and will stop the bot gracefully.
    updater.idle()


if __name__ == '__main__':
    main()
