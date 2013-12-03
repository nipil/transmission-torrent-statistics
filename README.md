## About

A QT-based daemon polling transmission instances via RPC at regular intervals. Fetches cumulated upload and download amounts for each torrent. Stores these information in a database for later use.

## Build prerequisite

On Debian-like systems
- qt-sdk
- libsqlite3
- libqjson-dev

## Configuration

First run creates a default configuration in `~/.config/nipil/transmission-torrent-statistics.conf`. It can be edited while the service is running and you can reload the configuration via SIGHUP.

## Data stored

In a master table
- torrent hash
- torrent name

In a dedicated table for each torrent
- time_t (number of seconds since epoch)
- downloadedEver (bytes downloaded)
- uploadedEver (bytes downloaded)

The amount value are the data collected by transmission, stored "raw" and unmodified.

## Irregularities in the data 

If you delete a torrent, and later add it again, and/or your transmission client cleared it's stats, then the "amount" values will not be continuous.

If the transmission client is unreachable at the time of the polling, the collected data will not be continuous.

These kind of issues need to be takent into account for every use of the collected data.

## Planned features

Integrate a basic web server to visualize collected data.
