function tts_init() {
    $("#tabs").tabs()
    $("#torrent_list_items_reload").click(function () {
        tts_torrent_list_reload()
        console.log("tts_torrent_list_reload requested")
    })
    tts_torrent_list_reload()
}

function tts_curtime() {
    var d = new Date()
    return Math.floor(d.getTime() / 1000)
}

function tts_agetime(cur, target) {
    var diff = Math.abs(cur - target)
    var unit = "sec"
    if (diff > 60 && unit == "sec") {
        diff /= 60
        unit = "min"
    }
    if (diff > 60 && unit == "min") {
        diff /= 60
        unit = "hour"
    }
    if (diff > 24 && unit == "hour") {
        diff /= 24
        unit = "day"
    }
    if (diff > 365 && unit == "day") {
        diff /= 365
        unit = "year"
    }
    var plur = ""
    if (diff >= 2)
        plur = "s"
    return Math.floor(diff) + " " + unit + plur
}

$.tablesorter.addParser({
                            id: 'attr_time',
                            is: function (s) {
                                return false
                            },
                            format: function (value, table, cell) {
                                return $(cell).attr('time')
                            },
                            type: 'numeric'
                        })

function tts_torrent_list_reload() {
    $.getJSON("/json/list", function (data) {
        var tdata = $("#torrent_list_items").append($("toto"))
        var ct = tts_curtime()
        console.log()
        $.each(data, function (key, val) {
            var row = $("<tr/>")
            var t_a = $("<td/>", {
                            time: val.last,
                            html: tts_agetime(ct, val.last)
                        })
            t_a.tooltip()
            row.append(t_a)
            var t_n = $("<td/>", {
                            title: val.hash,
                            html: val.name
                        })
            row.append(t_n)
            tdata.append(row)
        })
        $("#torrent_list").tablesorter({
                                           headers: {
                                               0: {
                                                   sorter: 'attr_time'
                                               }
                                           },
                                           sortList: [[0, 1]]
                                       })
    })
}
