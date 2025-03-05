var Width = 600;

function autosize (f) {
  if (f.scrollHeight > f.clientHeight) {
    f.style.height = f.scrollHeight + "px";
  }
}

// Draw a single bar chart representing
// results for one platform and architecture
function draw_chart(ctx, title, rows, mbsMax) {
    var xMax = Math.max(...Object.values(rows));

    // gray separator
    ctx.beginPath();
    ctx.strokeStyle = "#A0A0A0";
    ctx.moveTo(0, 0.5);
    ctx.lineTo(Width, 0.5);
    ctx.stroke();
    ctx.translate(0, 28);

    // title
    ctx.textAlign = "left";
    ctx.fillStyle = "#000000";
    ctx.font = 'bold 16px sans-serif';
    ctx.fillText(title, 0, 0);
    ctx.translate(0, 26);

    // chart rows
    Object.entries(rows).forEach(([name, mbs]) =>
    {
        var dy = 4;

        ctx.textAlign = "left";
        ctx.fillStyle = "#000000";
        ctx.font = '14px sans-serif';
        ctx.fillText(name, 0, 0);

        var xMbs = 195;
        if(name.startsWith("boost"))
            ctx.fillStyle = "#eb342a";
        else
            ctx.fillStyle = "#909090";
        var w = (Width - 170) * mbs / mbsMax
        if(w < xMbs - 120)
            w = xMbs - 120;
        ctx.fillRect(130, -20+dy, w, 20);

        ctx.textAlign = "right";
        ctx.font = '13px sans-serif';
        ctx.fillStyle = "#000000";
        ctx.fillText(mbs + " MB/s", xMbs + 1, 1);
        ctx.fillStyle = "#ffffff";
        ctx.fillText(mbs + " MB/s", xMbs, 0);

        ctx.translate(0, 24);
    });

    // gray separator
    ctx.translate(0, -4);
    ctx.beginPath();
    ctx.strokeStyle = "#A0A0A0";
    ctx.moveTo(0, 0.5);
    ctx.lineTo(Width, 0.5);
    ctx.stroke();
}

// Draw multiple bar charts, one
// for each platform and architecture
function draw_charts(ctx, tab, mbsMax) {
    // top margin
    ctx.translate(0, 16);

    // draw each chart
    Object.entries(tab).forEach(([name, rows]) =>
    {
        draw_chart(ctx, name, rows, mbsMax);
    });

    // bottom margin
    ctx.translate(0, 16);
}

function measure_and_draw(canvas, tab) {
    mbsMax = 0;
    Object.entries(tab).forEach(([name, rows]) =>
    {
        mbsMax = Math.max(mbsMax, ...Object.values(rows));
    });

    var LeftMargin = 48;

    // Measure the height needed
    canvas.width = Width;
    canvas.height = 30000;
    var ctx = canvas.getContext("2d");
    ctx.save();
    draw_charts(ctx, tab);
    var tx = ctx.getTransform();
    var height = tx["f"];

    // Resize the canvas to fit
    canvas.width = LeftMargin + Width + 20;
    canvas.height = height;

    // Make everything opaque
    ctx.fillStyle = "#FFFFFF";
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    ctx.translate(LeftMargin, 0);
    draw_charts(ctx, tab, mbsMax);
    ctx.restore();
}

Array.prototype.forEach.call(
    document.getElementsByClassName("simplesect"),
    (elem) => { elem.remove(); }
);

var data = document.getElementById("benchmarks-data");
var lines = data.innerHTML.split('\n');
var tab = new Object();
for (var i = 0; i < lines.length; i++)
{
    if(lines[i].length == 0)
        continue;
    items = lines[i].split(',');
    var title = items[0]
    var target = items[1];
    var name = items[2];
    var mbs = items[3];
    if(! (title in tab))
        tab[title] = new Object();
    if(! (target in tab[title]))
        tab[title][target] = new Object();
    var set = tab[title][target];
    if(! (name in set))
        set[name] = new Object();
    set[name] = mbs;
}

var benchmarks = document.getElementById("benchmarks");
benchmarks.innerHTML = "";
Object.entries(tab).forEach(([title, charts]) =>
{
    var tab_id = title.replace(/\s+|\./g, "_").toLowerCase();
    benchmarks.innerHTML +=
        "<h4>" + title + "</h4>" +
        "<canvas id=\"" + tab_id + "\" width=\"800\" height=\"600\">";
});
Object.entries(tab).forEach(([title, charts]) =>
{
    var tab_id = title.replace(/\s+|\./g, "_").toLowerCase();
    measure_and_draw(document.getElementById(tab_id), charts)
});
