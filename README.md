# Message Relay for Tox

Forwards all incoming messages to another Tox address, storing any messages when you're offline.

Currently only allows for forwaring to a single address.

Can toggle the relay to only store messages for when you're offline as opposed to actively relaying messages when you're online as well.

## Installation

To install on Debian:

    apt-get install build-essential libncurses5-dev libsodium-dev
    make
    make install

## Commands

    /help - Prints this message
    /id - Print the tox ID of this relay
    /name - Sets your name
    /addfriend <Tox ID> - Adds Tox id as a friend
    /addrelay <Tox ID> - Adds Tox id as message destination (and as a friend)
    /offlineonly <1|0> - 1 will cause the relay to only store messages. Default 0
    /clear - Clears the screen
    /quit - Exits the relay client
