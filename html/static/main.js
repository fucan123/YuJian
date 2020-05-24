var layer = layui.layer;
var form = layui.form;
var table = layui.table;
var $ = layui.$;
var client_width = document.documentElement.clientWidth;
var client_height = document.documentElement.clientHeight;

// 是否安装了dll驱动
var g_install_dll = false;
// 设置
var g_setting = []; 
// 打开菜单的数据索引
var g_open_menu_index = -1;
// 是否已开启喊话CALL
var g_open_talk = false;
// c++类对象
var cpp_obj = null, _c = null;

//layer.alert("加载完成.", { icon: 1 });
//ShowMsg("msg", null, 2);

function test(v, v2) {
    //alert(v+v2);
}

$(document).ready(function () {
    //ShowMsg(CallCpp(2, 3));
    $("#full").css("height", client_height + "px");
    setTimeout(function () {
        if (!cpp_obj) {
            fff();
            if (0)
                ShowMsg('无法获取到c++类对象, 程序可能无法继续, 请重新打开本程序.', '提示', 2);
        }
            
    }, 500);
});

function fff() {
    var zoom = detectZoom();
    if (client_height < $("body").height()) {
        var b_height = $("body").height();
        var bl = (b_height - client_height) / b_height * 100;
        bl = bl.toFixed(2);
        var msg = "页面可能未显示完全(当前页面高度" + b_height + ", 可视区域高度" + client_height+", 不可见比例"+bl+"%),";
        msg += "当前缩放比列(" + zoom + "%), 请尝试按(Ctrl+鼠标滚轮)缩放页面.";
        //alert(msg);
    }
    $("body").on("mousedown", function (event) {
        event = event || window.event;
        if (cpp_obj && event.button == 2) {
            document.oncontextmenu = function (event) {
                if (typeof event.returnValue !== 'undefine')
                    event.returnValue = false;
                if (typeof event.preventDefault !== 'undefine')
                    event.preventDefault();
            }
        }
    });

    $("#full").click(function () {
        if (!$("#more_data").is(":hidden")) {
            $("#more_data").hide();
            $("#full").hide();
        }
    });

    $("main").click(function () {
        if (parseInt($("#log").data('visable')) == 1) {
            ShowLog('log', 'r_tool', 'log_ul', 1);
        }

        $("#table_1_menu").fadeOut(500);
        g_open_menu_index = -1;
    });

    $(".tips").on("mouseover", function (event) {
        event = event || window.event;
        var left = $(this).offset().left;
        var top = $(this).offset().top;
        
        //alert(height);
        
        $("#tips_box").html($(this).data("text"));

        var width = $("#tips_box").width();
        var height = $(this).height();
        if ((left + width) >= client_width) {
            left = client_width - width - 30;
        }

        $("#tips_box").css({ "left": left, "top": event.pageY - 45 });
        $("#tips_box").show();
        

    });
    $(".tips").on("mouseout", function () {
        $("#tips_box").fadeOut(200);
    });

    $(".setting_radio input").on("change", function () {
        CallCpp("put_setting", $(this).attr("name"), parseInt($(this).val()), 0);
    });

    $(".my_ck_box").on("click", function () {
        var box = $(this).children("i");
        var text = box.text();

        var checked = 0;
        if (text.length) {
            box.html("");
            checked = 0;
        }
        else {
            box.html("&#xe605;");
            checked = 1;
        }

        CallCpp("put_setting", box.data("key"), checked, 0);
    });

    $(".menu li").on("mouseover", function () {
        if (!$(this).hasClass("disabled"))
            $(this).css("background", "#eeeeee");
    });
    $(".menu li").on("mouseout", function () {
        if (!$(this).hasClass("disabled"))
            $(this).css("background", "#ffffff");
    });
    $(".menu li").click(function () {
        if (!$(this).hasClass("disabled")) {
            var parent_id = $(this).parent("ul").attr("id");
            //CallCpp("select_menu", parent_id, g_open_menu_index, parseInt($(this).index()));
            var index = parseInt($(this).index());
            if (index == 0) {
                if (!g_install_dll && !Start())
                    return;

                if (!$("#agreement")[0].checked) {
                    ShowMsg("请同意用户协议, 如有异议请关闭本程序.", "提示", 2);
                    return;
                }
                CallCpp("open_game", g_open_menu_index);
            }
            if (index == 1) {
                CallCpp("close_game", g_open_menu_index);
            }
            if (index == 2) {
                CallCpp("in_team", g_open_menu_index);
            }

            $(this).parent("ul").fadeOut(300);
            //g_open_menu_index = -1;
        }
    });

    
    $("#table_1 tr").on("mouseover", function () {
        $(this).css("background", "#eeeeee");
    });
    $("#table_1 tr").on("mouseout", function () {
        $(this).css("background", "#ffffff");
    });
    $("#table_1 tr").on("mousedown", function (event) {
        event = event || window.event;
        if (event.button == 2) {
            if (!$(this).children("td:eq(0)").html()) {
                $("#table_1_menu").fadeOut(500);
                return;
            }

            g_open_menu_index = parseInt($(this).index());
            if (CallCpp("open_menu", parseInt($(this).index()))) {
                SetMenuDisabled("table_1_menu", 0, 1);
                SetMenuDisabled("table_1_menu", 1, 0);
                SetMenuDisabled("table_1_menu", 2, 0);
            }
            else {
                SetMenuDisabled("table_1_menu", 0, 0);
                SetMenuDisabled("table_1_menu", 1, 1);
                SetMenuDisabled("table_1_menu", 2, 1);
            }

            $("#table_1_menu").css({ "margin-left": (event.clientX + 10) + "px", "margin-top": (event.clientY + 5) + "px" });
            $("#table_1_menu").fadeIn(500);
        }
    });

    $("#table_1 .layui-form-checkbox").on("click", function () {
        var index = $(this).data('index');
        if (!index)
            return;

        if ($(this).hasClass('layui-form-checked')) {
            CallCpp("put_setting", "account_locklogin|" + index, 1);
            $(this).removeClass('layui-form-checked');
        }
        else {
            CallCpp("put_setting", "account_locklogin|" + index, 0);
            $(this).addClass('layui-form-checked')
        }
    });
}

function detectZoom() {
    var ratio = 0,
        screen = window.screen,
        ua = navigator.userAgent.toLowerCase();

    if (window.devicePixelRatio !== undefined) {
        ratio = window.devicePixelRatio;
    }
    else if (~ua.indexOf('msie')) {
        if (screen.deviceXDPI && screen.logicalXDPI) {
            ratio = screen.deviceXDPI / screen.logicalXDPI;
        }
    }
    else if (window.outerWidth !== undefined && window.innerWidth !== undefined) {
        ratio = window.outerWidth / window.innerWidth;
    }

    if (ratio) {
        ratio = Math.round(ratio * 100);
    }

    return ratio;
};

// 弹框
function ShowMsg(text, t, _icon) {
    if (!t)
        t = '信息';

    layer.open({
        icon: _icon,
        title: t,
        content: text,
        yes: function (index) {
            if (text.indexOf("请先激活") > -1)
                VerifyCard();
            layer.close(index);
        }
    }); 
}

// 显示工具栏
function ShowTool(id, sub_top) {
    return;
    var top = client_height - (client_height / 3);
    $("#" + id).show();
    $("#" + id).css({ "display": "block", "margin-top": (top - sub_top) + "px" });
}

// 设置元素宽度为可视宽度和在右边隐藏
function SetMarginLeftClientWidth(id) {
    $("#" + id).css({ "width": (client_width - 60) + "px", "margin-left": (client_width + 20) + "px" });
}

// 设置元素离顶部距离为可视区域高度
function SetMarginTopClientHeight(id, sub_top) {
    $("#" + id).css({ "margin-top": (client_height - sub_top) + "px" });
}

// 设置元素为可视高度
function SetClientHeight(id, sub_top) {
    $("#" + id).css("height", client_height - sub_top);
}

// 设置class
function SetClass(id, cla, rm_cla) {
    if (rm_cla)
        $("#" + id).removeClass(rm_cla);
    if (cla)
        $("#" + id).addClass(cla);
}

// 设置元素文字
function SetText(id, text) {
    $("#" + id).html(text);
    if (id == 'fb_count') {
        $('#fb_count_log').html(text);
    }
}

// 菜单禁用设置
function SetMenuDisabled(id, row, v) {
    if (v)
        $("#" + id).children("li:eq(" + row + ")").addClass("disabled");
    else
        $("#" + id).children("li:eq("+row+")").removeClass("disabled");
}

// 按钮禁用
function SetBtnDisabled(id, v, text) {
    if (v)
        $("#" + id).addClass("layui-btn-disabled");
    else
        $("#" + id).removeClass("layui-btn-disabled");

    if (text)
        $("#" + id).html(text);
}

// 添加一行表格
function AddTableRow(id, col_count, key, v) {
    if (!col_count)
        col_count = $("#" + id).children("colgroup").children("col").length;

    var length = $("#" + id).children("tbody").children("tr").length;
    var text = "<tr data-index=" + length + ">";
    for (var i = 0; i < col_count; i++) {
        text += "<td>";
        if (i == 0 && key) {
            text += key;
        }

        if ((i + 1) === col_count && key) {
            var disabled = length === 0 ? 'disabled' : '';
            //text += '<input type="checkbox" value="1" checked ' + disabled + ' />';
            text += '<div class="layui-unselect layui-form-checkbox layui-form-checked" lay-skin="primary" data-index=' + length + '><i class="layui-icon layui-icon-ok"></i></div>';
        }
        else {
            if (i > 0 && v) {
                text += v;
            }
        }
        

        text += "</td>";
    }
    text += "</tr>";

    $("#" + id).children("tbody").append(text);
}

// 自动填充表格行以达到美观
function FillTableRow(id, row_count, col_count) {
    if (!id)
        id = "table_1";
    if (!row_count) {
        var table_height = $("#" + id).height();
        var tr_height = table_height ? $("#" + id).children("tbody").find("tr:eq(0)").height() : 30;
        var div_height_p = $("#" + id).parent("div").height();
        row_count = parseInt((div_height_p - table_height) / tr_height); 
    }
    if (!col_count)
        col_count = $("#" + id).children("colgroup").children("col").length;

    for (var i = 0; i < row_count; i++) {
        AddTableRow(id, col_count, 0);
    }
}

// 更新表格列表数据
function UpdateTableText(id, row, col, text) {
    $("#"+id).children("tbody").find("tr:eq("+row+")").find("td:eq("+col+")").html(text);
}

// 更新表格class
function UpdateTableClass(id, row, col, cla, add) {
    var obj = $("#" + id).children("tbody").find("tr:eq(" + row + ")");
    if (col > -1)
        obj = obj.find("td:eq(" + col + ")");

    if (add)
        obj.addClass(cla);
    else
        obj.removeClass(cla);
}

// 更新表格样式
function UpdateTableStyle(id, row, col, key, value) {
    var obj = $("#" + id).children("tbody").find("tr:eq(" + row + ")");
    if (col > -1)
        obj = obj.find("td:eq(" + col + ")");

    obj.css(key, value);
}

var log_num = 0;
// 添加日记
function AddLog(id, str, cla) {
    if (!id)
        id = 'log_ul';

    var first_enter = str[0] == "\n" ? true : false;
    var end_enter = str[str.length - 1] == "\n" ? true : false;

    str = str.replace("\n", "");

    var text = '';
    if (first_enter) text += '<li>----------------------------------------------------------------</li>';

    text += "<li>";
    if (cla)
        text = '<li class="' + cla + '">';

    log_num++;

    var d = new Date();
    text += "<span style='float:right;'>[" + AddZero(d.getMonth() + 1) + "-" + AddZero(d.getDate()) + " ";
    text += AddZero(d.getHours()) + ":" + AddZero(d.getMinutes()) + ":" + AddZero(d.getSeconds());
    text += "]</span>";
    text += "<span>" + log_num + ".</span> ";
    text += str;
    text += "</li>";

    if (end_enter) text += '<li>----------------------------------------------------------------</li>';

    $('#' + id).append(text);
    if (log_num > 500) {
        $('#' + id).children("li:eq(0)").remove();
    }
    if ((log_num % 3000) == 0)
        RandTitle();

    $('#log_ul').scrollTop($('#log_ul')[0].scrollHeight);
}

// 显示日记元素
function ShowLog(id, text_id, log_id, hide) {
    if (!id)
        id = "#log";
    else
        id = '#' + id;

    var text = '';
    if (!hide && parseInt($(id).data('visable')) == 0) {
        text = '隐<br />藏<br /><i class="layui-icon">&#xe603;</i><br />日<br />记<br />';
        $(id).data('visable', 1);
        //$(id).css("display", 'block');
        $(id).fadeIn(500);
        //$(id).animate({ "width":700 }, 100);

        $('#' + log_id).scrollTop($('#' + log_id)[0].scrollHeight);
    }
    else {
        text = '显<br />示<br /><i class="layui-icon">&#xe602;</i><br />日<br />记<br />';
        $(id).data('visable', 0);
       // $(id).css("display", 'none');
        $(id).fadeOut(500);
        //$(id).animate({ "width": 0 }, 150);
    }

    $('#' + text_id).html(text);
}

// 更新状态文字
function UpdateStatusText(text, flag) {
    $("#status").show();

    var icon = '';
    if (flag == 1)
        icon = 'xe645';
    else if (flag == 2) // 勾
        icon = 'xe605';
    else if (flag == 3) // ×
        icon = 'x1006';
    else if (flag == 4) // 加载
        icon = 'xe63d';
    else if (flag == 5) // 水滴
        icon = 'xe636';

    if (icon)
        icon = '<i class="layui-icon">&#' + icon + ';</i> ';

    var d = new Date();
    text += " <span class=' cb'>[" + (d.getFullYear()) + "-" + AddZero(d.getMonth() + 1) + "-" + AddZero(d.getDate()) + " ";
    text += AddZero(d.getHours()) + ":" + AddZero(d.getMinutes()) + ":" + AddZero(d.getSeconds());
    text += "] </span>";
    $('#status_text').html(icon+text);
}

// 显示设置 
function ShowSetting(id, h) {
    ShowSettingCallBack();
    $("#full").show();
    $("#" + id).css("margin-top", ((client_height - h) / 2 - 50)+"px");
    $("#" + id).show();
}

// 确认设置
function SaveSetting(id) {
    PutSetting(id);
    UpdateStatusText("设置更新成功.", 2);
    ShowMsg("设置成功.", "提示", 1);

    if (id == "talk_setting") {
        if (g_setting['talk_open'] && !g_open_talk) {
            AutoTalk();
        }
    } 

    HideSetting();
}

// 隐藏设置
function HideSetting() {
    $(".setting_div").hide();
    $("#full").hide();
}

// 自动喊话CALL
function AutoTalk() {
    if (!g_setting['talk_open'] || !g_setting['talk_content']) {
        g_open_talk = false;
        return;
    }

    g_open_talk = true;
    AddLog("log_ul", '喊话:'+g_setting['talk_content'], "blue");
    CallCpp("talk", g_setting['talk_content'], parseInt(g_setting['talk_type']));
    setTimeout(AutoTalk, g_setting['talk_intval'] * 1000);
}

function ShowSettingCallBack() {
    //setTimeout(function () {
    for (var key in g_setting) {
        SetSetting(key, g_setting[key], true);
    }
    //}, 20);
}

// 获得设置
function SetSetting(name, v, show) {
    if (typeof show === 'undefined' || !show) {
        g_setting[name] = v;
        return;
    }
    
    var obj = $(".setting_div input[name=" + name + "]");
    if (!obj.length) {
        obj = $(".setting_div select[name=" + name + "]");
        if (!obj.length)
            return;
    }
    if (obj[0].type == 'checkbox') {
        obj.attr('checked', v ? true : false);
    }
    else if (obj[0] == 'select') {
        $(obj).children('option').each(function () {
            if (this.value == v)
                this.selected = true;
        });
    }
    else {
        obj.val(v);
    }
}

// 推送设置
function PutSetting(id) {
    $('#'+id+' input').each(function () {
        var value = $(this).data("string") ? this.value : parseInt(this.value);
        if (this.type == 'checkbox') {
            value = this.checked ? 1 : 0;
        }
        SetSetting(this.name, value);

        CallCpp("put_setting", this.name, value, parseInt($(this).data("string")));
    });
    $('#' + id +' select').each(function () {
        var value = $(this).data("string") ? this.value : parseInt(this.value);
        SetSetting(this.name, value);

        CallCpp("put_setting", this.name, value, parseInt($(this).data("string")));
    });
}

// 移至本机
function YZBJ(obj) {
    var value = $('#card_input').val();
    if (value.length == 0) {
        layer.msg("请输入卡号.");
        $('#card_input').focus();
        return;
    }
    if (value.length != 32) {
        layer.msg("卡号不正确, 请认真核对(确认是否误输入了空格).");
        $('#card_input').focus();
        return;
    }

    layer.confirm("您确定把此卡移到本机吗？如若这样做，原来机器剩余时间会转移到本机器<br />(第<b class='red'>1</b>次会扣除<b class='red'>3</b>天, 此后每移<b class='red'>1</b>次会扣除<b class='red'>已移机次数*2+3</b>天)<br />是否确定继续？", {
        icon: 3,
        title: '提示',
        btn: ['确定', '取消']
    }, function () {
        CallCpp("getin_card", value);
    }, function () {
        // 按钮2的事件
    });
}

// 激活
function VerifyCard(obj) {
    var value = $('#card_input').val();
    if (value) {
        if (value.length != 32) {
            layer.msg("卡号不正确, 请认真核对(确认是否误输入了空格).");
            $('#card_input').focus();
            return;
        }

        SetBtnDisabled("card_btn", 1, "确认中...");
        CallCpp("verify_card", value);
        SetBtnDisabled("card_btn", 0, "确定");
    }
    HideSetting(); 
}

// 成功
function VerifyOk() {
    //Start();
}

// 更新
function UpdateVer(obj) {
    /*
    ShowMsg("目前已暂停自动更新, 敬请谅解.", "已暂停更新", 5);
    return;
    */

    if ($('#update_btn').hasClass('update_btn_ing'))
        return;
        
    $('#update_btn').removeClass('update_btn');
    $('#update_btn').addClass("update_btn_ing");
    $('#update_btn').html('更新中...');

    CallCpp("update_ver");
}

// 更新完成
function UpdateVerOk() {
    $('#update_btn').removeClass('update_btn_ing');
    $('#update_btn').addClass("update_btn");
    $('#update_btn').html('检查更新<i class="layui-icon">&#xe607;</i>');
    UpdateStatusText("更新完成.", 2);
}

// 更新流程
function UpdateStep(obj) {
    if ($('#step_btn').hasClass('step_btn_ing'))
        return;

    layer.confirm("同步流程会覆盖同名文件, 请您先手动备份好, 是否还继续?", {
        icon: 3,
        title: '提示',
        btn: ['确定', '取消']
    }, function () {
        $('#step_btn').removeClass('step_btn');
        $('#step_btn').addClass("step_btn_ing");
        $('#step_btn').html('同步中...');
        layer.closeAll();

        CallCpp("update_step");
    }, function () {
        // 按钮2的事件
    });
}

// 更新流程完成
function UpdateStepOk() {
    $('#step_btn').removeClass('step_btn_ing');
    $('#step_btn').addClass("step_btn");
    $('#step_btn').html('同步流程<i class="layui-icon">&#xe607;</i>');
    UpdateStatusText("同步流程完成.", 2);

    ShowMsg("同步流程已完成, 但需要重启软件生效.", "提示", 1);
}

// 前面添加零
function AddZero(v) {
    if (parseInt(v) < 10)
        v = '0' + v;
    return v;
}

var fb_records = [];
// 详细数据
function MoreData(h) {
    //alert((new Date('2020-1-13').getTime() / 1000) + " " + (new Date().getTime() / 1000));
  //  alert(getTodayTime());
    //alert(getTodayTime() + 86400);
    $("#full").show();
    $("#more_data").css("margin-top", ((client_height - h) / 2 - 50) + "px");
    $("#more_data").show();

    fb_records = CallCpp("fb_record");
    ShowMoreDataNum();
}

function ShowMoreDataNum() {
    var index = 0, length = fb_records.length;
    var text = '';
    for (var day = 0, key = 1; day > -30; day-- , key++) {
        var s_time = getDayTime(day);
        var e_time = s_time + 86400;
        //if (fb_records[0].start_time > e_time)
        //    break;

        var num = 0, num_reborn = 0, num_out = 0;
        for (; index < length;) {
            var data = fb_records[index];
            if (data.start_time < s_time)
                break;

            if (data.start_time >= s_time && data.start_time < e_time) {
                if (data.status === 0)
                    num++;
                if (data.status === 1)
                    num_reborn++;
                if (data.status === 2)
                    num_out++;
            }

            index++;
        }
        if (1 || num || num_reborn || num_out) {
            text += '<div class="num">';
            text += '<span class="d" >' + timestampToTime(s_time, true) + '</span>';
            text += '<span class="ml10 c3">通关次数:(' + num + ')</span>';
            if (num_reborn)
                text += '<span class="ml10 orange">复活重开:(' + num_reborn + ')</span>';
            if (num_out)
                text += '<span class="ml10 red">超时重开:(' + num_out + ')</span>';
            text += '<a href="javascript:ShowDataDetail(1, ' +s_time+');" class="ml5">详细</a></div> ';
        }
    }
    $('#date_num').html(text);
    ShowDataDetail(0);
}

function HideMoreData() {
    $("#full").hide();
    $("#more_data").hide();
}

function ShowDataDetail(v, s_time) {
    if (v) {
        $("#date_num").hide();
        $("#date_detail").show();

        var text = '', fast = 0, slow = 0, avg = 0;
        var e_time = s_time + 86400;
        var key = 1, out_num = 0;
        for (var i in fb_records) {
            if (fb_records[i].start_time < s_time)
                break;

            if (fb_records[i].start_time >= s_time && fb_records[i].start_time < e_time) {
                text += '<div class="detail">';
                text += '<span class="c0" style="display:inline-block;width:30px;">' + key+ '.</span>';
                text += '<span class="c3">'+format_fb_time(fb_records[i].start_time)+'</span>';
                text += ' 至 ';
                text += '<span class="c3">' + format_fb_time(fb_records[i].end_time) + '</span>';
                text += '<span class="ml10 c3 b">用时:' + FormatTimeLong(fb_records[i].time_long) + '</span>';

                if (fb_records[i].status === 1)
                    text += '<span class="ml5 orange b">(复活重开)</span>';
                else if (fb_records[i].status === 2)
                    text += '<span class="ml5 red b">(超时重开)</span>';

                text += '</div>';

                if (fb_records[i].status === 0) {
                    if (fast === 0) {
                        fast = fb_records[i].time_long;
                    }
                    else if (fb_records[i].time_long < fast) {
                        fast = fb_records[i].time_long;
                    }
                    if (slow < fb_records[i].time_long)
                        slow = fb_records[i].time_long;

                    avg += fb_records[i].time_long;
                }
                else {
                    out_num++;
                }

                key++;
            }
        }

        if (key > 1)
            avg /= (key - 1 - out_num);

        $('#date_detail .date').html(timestampToTime(s_time, true));
        $('#date_detail .fast').html('最快:'+FormatTimeLong(fast));
        $('#date_detail .slow').html('最慢:' +FormatTimeLong(slow));
        $('#date_detail .avg').html('平均:' +FormatTimeLong(parseInt(avg)));

        if (text === '') text = '<div>无数据</div>';
        $('#date_detail_list').html(text);
    }
    else {
        $("#date_detail").hide();
        $("#date_num").show();
    }
}

function getDayTime(day) {
    var d = new Date();
    var Y = d.getFullYear() + '-';
    var M = (d.getMonth() + 1) + '-';
    var D = d.getDate();
    d.setDate(D + day);
    d.setHours(0);
    d.setMinutes(0);
    d.setSeconds(0);
    return parseInt(new Date(d).getTime() / 1000);
}

function format_fb_time(timestamp) {
    var date = new Date(timestamp * 1000);
    var h = AddZero(date.getHours()) + '时';
    var m = AddZero(date.getMinutes())+'分';
    var s = AddZero(date.getSeconds())+'秒';
    return  h + m + s;
}

// 时长转成文字
function FormatTimeLong(time_long) {
    var text = '';

    text += AddZero(parseInt((time_long % 3600) / 60)) + '分';
    text += AddZero(parseInt(time_long % 60)) + '秒';
    return text;
}

function timestampToTime(timestamp, only_date, only_time) {
    var Y = '', M = '', D = '';
    var date = new Date(timestamp * 1000);//时间戳为10位需*1000，时间戳为13位的话不需乘1000
    if (!only_time) {
        var Y = date.getFullYear() + '-';
        var M = AddZero(date.getMonth() + 1) + '-';
        var D = AddZero(date.getDate());
    }
   
    if (only_date)
        return Y + M + D;
    else
        D += ' ';
    var h = date.getHours() + ':';
    var m = AddZero(date.getMinutes());
    var s = AddZero(date.getSeconds());
    return Y + M + D + h + m + s;
}

function Start() {
    if (!$("#agreement")[0].checked) {
        ShowMsg("请同意用户协议, 如有异议请关闭本程序.", "提示", 2);
        return;
    }

    g_install_dll = false;
    var result = CallCpp('start');
    if (result === -1) {
        ShowMsg("请先激活.", "提示", 2);
        return false;
    }
    if (result === 0) {
        ShowMsg("启动失败, 原因请看日记.", "提示", 2);
        return false;
    }

    if ($('#start_btn').html() == '启动')
        $('#start_btn').html('停止');
    else
        $('#start_btn').html('启动');

    g_install_dll = true;
    return true;
}

function AutoPlay() {
    if (!$("#agreement")[0].checked) {
        ShowMsg("请同意用户协议, 如有异议请关闭本程序.", "提示", 2);
        return;
    }

    var index;
    if ($('#auto_play_btn').html() == '自动登号') {
        if (!g_install_dll && !Start())
            return;

        $('#auto_play_btn').html('停止登号');
        index = -1;
    }
    else {
        if (g_install_dll) Start();

        $('#auto_play_btn').html('自动登号');
        index = -2;
    }
    CallCpp("open_game", index, 0);
}

function RandTitle() {
    var data = [];
    for (var i = 48; i <= 57; i++) {
        data.push(i);
    }
    for (var i = 65; i <= 90; i++) {
        data.push(i);
        data.push(i + 32);
    }

    var title = '';
    var length = Math.floor(Math.random() * (16 - 8)) + 8;
    for (var i = 0; i <= length; i++) {
        var index = Math.floor(Math.random() * data.length);
        title += String.fromCharCode(data[index]);
    }
    CallCpp("set_title", title);
}

function ShowAgreement() {
    layer.open({
        title: "用户服务协议",
        area: ['500px', '500px'],
        content: $("#agreement-content").html()
    }); 
}


// 设置c++类对象
function SetCppObj(obj) {
    cpp_obj = obj;
    _c = cpp_obj;
    fff();
}