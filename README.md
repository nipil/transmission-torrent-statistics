## About

A daemon polling transmission P2P client at regular intervals to gather statistics. Fetches cumulated upload amounts for each torrent. Stores these information in a database for later use. Proposes a dynamic web portal to show the resulting data as graphs.

## Program Dependencies

On Debian-like systems where you build it
- qt-sdk
- libqjson-dev

On Debian-like systems where you run it
- libqt4-network
- libqt4-sql-sqlite
- libqjson0

## Web portal dependencies

As of today, the following modules are used
- http://jquery.com (MIT licence)
- http://jqueryui.com (MIT licence)
- https://github.com/Mottie/tablesorter/ (MIT or GPL licence)
- http://www.flotcharts.org (MIT License)

These modules are served locally (see path names for version numbers)

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

### DB Maintenance

As the collected data grows, database optimisation and maintenance operation became a requirement

When any mainteance operation is requested, the following happens :
- db maintenance is triggered on startup, but before the main loop
- the current database is moved out of the way (renamed with a timestamp prefix)
- a fresh, empty database is created
- for each known torrent in the old database
- for each sample in the torrent's table
- apply requested maintenance operations on the sample
- store the sample (if applicable) into the new database

This has two advantages over working on one single database
- the old db is backed up, in case anything goes wrong
- the tables are re-created, which is the only way to alter table structure
- removing records from an sqlite db doesn't really reduce file size

DB Maintenance is not something that should be run often, and in fact, should "rarely" be run, except for version upgrades requiring it, or to keep storage size under control.

### Data deduplication

For versions 1.0 to 1.2, each and every sample was stored in the database. Since then, a data deduplication algorythm has been implemented which stored the current sample only if there was a change since the last seen one (wether it be download or upload amount). As long as there's no activity on a specific torrent, no data is stored.

As a consequence, a `--db-deduplication` command line option has been introduced, to trim existing databases. The program continues to work with untrimmed databases, but given that a single maintenance run will remove all duplicates forever (and no new duplicates will be added by the current code), all databases created before 1.3 *should* undergo maintenance. Database create later don't need this kind of maintenance at all.

### Data age-based cleanup

Versions 1.4 introduces a new DB maintenance operation : sample suppression based on sample age. This is done through option `db-age-cleanup=N` where `N>1` represent the maximum tolerated age for a sample, expressed in days. Each and every sample older than N days will be removed from the database, and torrents where no sample are present are removed from the database during maintenance.

If storage is an issue, the user can use this option periodically. If storage is not a problem, you don't need to use this option at all.

## Irregularities in the data 

If you delete a torrent, and later add it again, and/or your transmission client cleared it's stats, then the "amount" values will not be solely increasing.

If the transmission client is unreachable at the time of the polling, or the data hasn't change since last known polling, no sample is stored, thus don't expect to get one sample per polling interval when querying the database.

These kind of issues need to be taken into account for every later use of the collected data.

