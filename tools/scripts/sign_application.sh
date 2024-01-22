#!/bin/bash

gpg --output "$1.sig" --detach-sig "$1"
gpg --verify "$1.sig" "$1"