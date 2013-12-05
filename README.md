## About

A QT-based daemon polling transmission P2P client at regular intervals to gather statistics.

Fetches cumulated upload and download amounts for each torrent.

Stores these information in a database for later use.

## Dependencies

On Debian-like systems where you build it
- qt-sdk
- libqjson-dev

On Debian-like systems where you run it
- libqt4-network
- libqt4-sql-sqlite
- libqjson

## Configuration

First run creates a default configuration in `~/.config/nipil/transmission-torrent-statistics.conf`.

It can be edited while the service is running and you can reload the configuration via SIGHUP.

## Web server

A very basic web server is integrated
- uses tcp port 4646 by default.
- see section `[WebServer]` of the configuration

### /json/list

Returns list of all torrents ever seen in the database (active or not). Example:

	[
	   {
	      "hash":"26942386b3e4e69fbe8bbd462d14076d17123456",
	      "name":"example_torrent-123 you like it"
	   },
	   ...
	]

### /json/$H/$TS/$TE

Returns transfer amounts for a specific torrent.

Details:
- `$H` is a hash taken from `/json/list`
- `$TS` is the start time of the search range (in sec, aka unix time)
- `$TE` is end time for the search range (same format).
- Only records satisfying `$TS < t < $TE` are returned.
- Results are sorted by increasing time (first record is older than last)

Example : `/json/26942386b3e4e69fbe8bbd462d14076123456789/1/3999999999`
  
	[
	   {
	      "d":"0",
	      "t":"1386104803",
	      "u":"2440873929"
	   },
	   ...
	]

### Any other URL

Other URL are treated as a local subpath from the base configured path, found files are served "as-is".

## Database

Storage is done in an SQlite database

In a master table
- hash (40 char hex string)
- name

In a dedicated table for each torrent
- unixtime (number of seconds since epoch)
- downloadedEver (bytes downloaded)
- uploadedEver (bytes downloaded)

The amount value are the data collected by transmission, stored "raw" and unmodified.

## Irregularities in the data 

If you delete a torrent, and later add it again, and/or your transmission client cleared it's stats, then the "amount" values will not be continuous.

If the transmission client is unreachable at the time of the polling, the collected data will not be continuous (ie there will be holes in the data)

These kind of issues need to be taken into account for every later use of the collected data.

## To be done

Dynamic web portal to visualize the data.
