## About

A QT-based daemon polling transmission instances via RPC at regular intervals to get cumulated upload and download amounts for each torrent, then stores these information in a database.

## Build prerequisite

On Debian-like systems
- qt-sdk
- libsqlite3
- libqjson-dev

## Configuration

First run creates a default configuration in ~/.config/nipil/transmission-torrent-statistics.conf

It can be edited while the service is running, then do a reload request via SIGHUP.

## Data stored

In a master table, store
- torrent hash
- torrent name

In a dedicated table for each torrent
- time_t (number of seconds since epoch)
- downloadedEver (bytes downloaded)
- uploadedEver (bytes downloaded)

## Planned features

Integrate a basic web server to visualize collected data.
