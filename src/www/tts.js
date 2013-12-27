// span time in seconds used for visualization
var chart_timespan
var chart_timemode
var chart_hash

// called when page is loaded
function tts_init() {
    // init tabs
    $("#tabs").tabs()

    // init radiobuttons in detailed tab
    $("#timespan").buttonset()
    $("#timespan > input").click(function () {
        chart_timespan = $(this).attr("amount")
        tts_draw_graph()
    })

    // initial selection
    $("#timespan1").click()

    // init radiobuttons in detailed tab
    $("#timemode").buttonset()
    $("#timemode > input").click(function () {
        chart_timemode = $(this).attr("mode")
        tts_draw_graph()
    })

    // initial selection
    $("#timemode1").click()

    // make table sortable
    $("#torrent_list").tablesorter({
                                       headers: {
                                           0: {
                                               sorter: 'attr_time'
                                           }
                                       },
                                       sortList: [[0, 1]]
                                   })

    // init torrent list reload (maybe replaced by timed reload ?
    $("#torrent_list_items_reload").click(function () {
        tts_torrent_list_reload()
    })
    // initial loading
    tts_torrent_list_reload()
}

function tts_curtime() {
    var d = new Date()
    return Math.floor(d.getTime() / 1000)
}

function tts_agetime(cur, target) {
    if (target == 0)
        return "never"
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

function tts_draw_graph_absolute(data) {
    var points = []
    $.each(data, function (key, val) {
        var x = parseInt(val.t)
        var y = parseInt(val.u)
        points.push([val.t, val.u])
    })
    var options = {
        series: {
            lines: {
                show: true
            },
            points: {
                show: true
            }
        }
    }
    var plot = $.plot($("#chart"), [points], options)
}

function tts_draw_graph_differential(data) {
    var points = []
    if (data.length < 2)
        return
    var last_t = parseInt(data[0].t)
    var last_u = parseInt(data[0].u)
    var delta = 0
    var i = 0
    $.each(data, function (key, val) {
        var cur_t = parseInt(val.t)
        var cur_u = parseInt(val.u)
        if (cur_t > last_t) {
            var du = cur_u - last_u
            if (du < 0)
                // stats were reset inbetween polls
                du = last_u
            var dt = cur_t - last_t
            var speed = du / dt
            // console.debug(i,cur_t, last_t, cur_u, last_u, du, dt, speed)
            points.push([cur_t, speed])
        }
        last_t = cur_t
        last_u = cur_u
        i++
    })
    var options = {
        series: {
            lines: {
                show: true
            },
            points: {
                show: true
            }
        }
    }
    var plot = $.plot($("#chart"), [points], options)
}

function tts_draw_graph() {
    // verifying hash
    if (!chart_hash)
        return

    // calculating display window
    if (!chart_timespan)
        chart_timespan = 60 * 60
    var timespan_end = tts_curtime()
    var timespan_start = timespan_end - chart_timespan

    // requesting data
    $.getJSON("/json/" + chart_hash + "/" + timespan_start + "/" + timespan_end,
              function (data) {
                  if (chart_timemode == 0)
                      tts_draw_graph_absolute(data)
                  else
                      tts_draw_graph_differential(data)
              })
}

function tts_set_graph(hash) {
    chart_hash = hash
    tts_draw_graph()
}

function tts_torrent_list_reload() {
    var tdata = $("#torrent_list_items")
    tdata.empty()
    $.getJSON("/json/list", function (data) {
        if (data.length == 0) {
            tdata.html("<tr><td>--</td><td>--</td></tr>")
            return
        }
        var ct = tts_curtime()
        $.each(data, function (key, val) {
            var row = $("<tr/>")
            var age = $("<td/>", {
                            time: val.last,
                            html: tts_agetime(ct, val.last)
                        })
            row.append(age)
            var link = $("<a/>", {
                             href: "#",
                             html: val.name
                         })
            link.click(function () {
                $("#tabs").tabs("option", "active", 1)
                $("#detailname").html(val.name)
                tts_set_graph(val.hash)
            })
            var name = $("<td/>", {
                             title: val.hash
                         })
            name.append(link)
            row.append(name)
            tdata.append(row)
        })
        $("#torrent_list").trigger("update")
    })
}
