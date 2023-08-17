# -*- coding: utf-8 -
"""
@author: KiWiCT
@desc: 本模块主要是用于和设备交互, 并向前端响应数据
@createDate: 2019/10/12
@updateDate: 2021/01/21
"""
import encodings.utf_8
import json
import os
import re
import shutil
import time
from datetime import datetime
from io import StringIO

import psutil
from flask import Blueprint, session, make_response, send_from_directory
from flask import abort
from flask import jsonify
from flask import request

from app.api.SocketUtil import GetAlarmMessage, SetParameterValues, GetPerformanceMessage, GetParameterValues, \
    get_response_data, get_overall_message, add_websocket_clients
from app.extensions import photos
from app.utils import FormatConversion, FileOperations, user_authenticate, tree_data_format, cookie_get, \
    MoreFormatConversion, base64_decode, file_to_list, verification_code, update_info, home_data, home_opt_status, \
    parameter_vilify, system_time, get_son_data, root_path, GetTemplateDirList, \
    get_PowersumeDataFromFile, VersionCheck, GenerateEUListFromCustom, \
    GenerateEUParamFromCustom, GenerateRUParamFromCustom, GetEUFromWhichWay, des_encrypt, des_descrypt, des_publicKey, \
    generator_user_id_number, LoginTimer, LoginTimerObj, login_requied, des_decrypt_form_js, invalid_character_escaped, \
    des_encrypt_form_js, generate_password, encrypt_password, html_filter_tags, set_session_time_out, \
    set_cookie_time_out, get_session_time_out, write_event_log_file, api_requied, csrf_handle, get_ipsec_ac_files, \
    del_ipsec_ac_files

# 创建蓝图
acs_data = Blueprint('data', __name__)
loginTimerObj = LoginTimerObj()


# 放置密钥
def push_encrypt_code(resp):
    session["deny"] = encrypt_password(generate_password(64))
    session["permit"] = encrypt_password(generate_password(64))
    session["access"] = encrypt_password(generate_password(64))
    resp.set_cookie('deny_to', session.get("deny"), httponly=True)
    resp.set_cookie('permit_to', session.get("permit"), httponly=True)
    resp.set_cookie('access_to', session.get("access"), httponly=True)


# 用户登录
@acs_data.route('/login', methods=["POST"])
def acs_login():
    login_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    user_ip = request.remote_addr
    session.permanent = False
    access_nums = '0'
    URL = request.url

    # 拿取请求数据
    request_params = request.json

    # 获取参数: 用户名, 密码, 验证码
    password = base64_decode(request_params.get("password"))
    username = request_params.get("username")
    vercode = request_params.get("vercode")
    code = session.get("verify_code")

    # 获取状态码和提示消息
    msg = cookie_get().get("msg", {})
    if not system_time():
        # write log
        write_event_log_file(login_time, "login", username, user_ip
                             + ', request URL: ' + URL + ', status code: 503  ............ [Failure]')
        return jsonify(msg.get("503", {}))

    # 校验请求数据
    if request_params is None or not isinstance(request_params, dict):
        # write log
        write_event_log_file(login_time, "login", username, user_ip
                             + ', request URL: ' + URL + ', status code: 402  ............ [Failure]')
        return jsonify(msg.get("402", {}))

    # 效验参数
    if not all([username, password, vercode]):
        # write log
        write_event_log_file(login_time, "login", username, user_ip
                             + ', request URL: ' + URL + ', status code: 402  ............ [Failure]')
        return jsonify(msg.get("402", {}))

    if vercode != code:
        # write log
        write_event_log_file(login_time, "login", username, user_ip
                             + ', request URL: ' + URL + ', status code: 507  ............ [Failure]')
        return jsonify({"code": 507, "msg": msg.get("code_error", {})})

    try:
        access_nums = session.get(str("access_nums_%s" % user_ip))
    except Exception as e:
        print(e)
    else:
        if access_nums is not None and int(access_nums) >= 5:
            session[str("access_nums_%s" % user_ip)] = "0"
            timer_handle = LoginTimer(60)
            timer_handle.start(user_ip)
            loginTimerObj.set_timer_handle(timer_handle)
            # write log
            write_event_log_file(login_time, "login", username, user_ip
                                 + ', request URL: ' + URL + ', status code: 505  ............ [Failure]')
            return jsonify(msg.get("505", {}))
        elif loginTimerObj.get_timer_handle() is not None and loginTimerObj.get_timer_handle().get_time_out() > 0:
            # write log
            write_event_log_file(login_time, "login", username, user_ip
                                 + ', request URL: ' + URL + ', status code: 505  ............ [Failure]')
            return jsonify(msg.get("505", {}))

    # 读取用户信息配置文件
    file_obj = FileOperations()
    user_list = file_obj.u  # type: list
    black_list = file_obj.black()
    user_list.extend(black_list)
    resp = ''

    # 判断用户名是否存在配置文件中
    if username in [item.get("username") for item in user_list]:
        for i in user_list:
            # 判断用户名是否相等
            if i.get("username") == username:
                # 判断用户密码是否一致
                if i.get("password") == des_encrypt(password, des_publicKey()):
                    # 密码一致, 将用户类型, 用户名, ID存入session中
                    session["usertype"] = i.get("usertype")
                    session["username"] = i.get("username")
                    session["id"] = i.get("id")
                    session[str("access_nums_%s" % user_ip)] = "0"

                    # 返回登录成功状态码
                    resp = jsonify({"code": 200, "msg": msg.get("login_success")})

                    # 设置cookie, 用户名和用户类型
                    resp.set_cookie('username', username, httponly=True)
                    resp.set_cookie('usertype', i.get("usertype"), httponly=True)
                    resp.set_cookie('id', des_encrypt(i.get("id"), des_publicKey()), httponly=True)

                    # write log
                    write_event_log_file(login_time, "login", username, '' + user_ip + ', request URL: '
                                         + URL + ', status code: 200  ............ [Success]')

                    push_encrypt_code(resp)

                    # 根据不同用户展示不同的logo和一些信息
                    update_info(i.get("changetype", {}))
                    break
                else:
                    # 密码错误, 提示用户密码错误
                    if access_nums is not None:
                        access_nums = str('%s' % (int(access_nums) + 1))
                    else:
                        access_nums = str('%s' % 1)
                    session[str("access_nums_%s" % user_ip)] = access_nums
                    resp = jsonify({"code": 400, "msg": msg.get("password_error"), "data": {}})

                    # write log
                    write_event_log_file(login_time, "login", username, user_ip
                                         + ', request URL: ' + URL + ', status code: 400  ............ [Failure]')
    else:
        # 用户名错误提示用户
        if access_nums is not None:
            access_nums = str('%s' % (int(access_nums) + 1))
        else:
            access_nums = str('%s' % 1)
        session[str("access_nums_%s" % user_ip)] = access_nums
        resp = jsonify({"code": 400, "msg": msg.get("username_error"), "data": {}})

        # write log
        write_event_log_file(login_time, "login", username, user_ip
                             + ', request URL: ' + URL + ', status code: 400  ............ [Failure]')

    return resp


# 生成验证码
@acs_data.route('/code', methods=['GET'])
def create_code():
    # 获取状态码和提示消息
    msg = cookie_get().get("msg", {})
    if not system_time():
        return jsonify(msg.get("503", {}))
    # 生成验证码
    verify_code = verification_code()
    # 将验证码存入session中
    session['verify_code'] = verify_code
    return jsonify(code=200, msg="OK", verifyCode=verify_code)


# 修改密码
@acs_data.route('/changePassword', methods=["POST"])
@login_requied
@user_authenticate("super")
def acs_change_password():
    """修改密码API"""
    # 拿取当前的语言提示字典
    msg = cookie_get().get("msg", {})
    if not system_time():
        return jsonify(msg.get("503", {}))
    # 获取请求数据
    params = request.json
    # 获取用户ID
    userid = params.get("user-list")
    # 获取选择要修改密码的用户名
    username = params.get("username")
    # 获取选择要修改密码的用户ID
    user_id = params.get("user_id")
    # 获取用户当前的密码
    old_password = params.get("oldPassword")
    # 获取用户新密码(第一次输入)
    first_password = params.get("firstPassword")
    # 获取用户新密码(再次输入)
    second_password = params.get("secondPassword")
    # 非空判断
    if not all([userid, username, user_id, old_password, first_password, second_password]):
        # 必填参数为空,终止请求,返回提示信息
        return jsonify(msg.get("402", {}))
    # 从session获取当前操作用户类型
    session_usertype = session.get('usertype')
    session_user_id = session.get('id')
    if not all([session_usertype]):
        return jsonify(msg.get("502", {}))
    # 判断第一次输入的密码和第二次输入的密码是否一致
    if first_password != second_password:
        return jsonify(code=400, msg=msg.get("password_different"))
    # 创建FileOperations实例对象
    file_obj = FileOperations()
    # 获取用户信息
    user_list = file_obj.u
    black_list = file_obj.black()  # type: list

    # 判断用户名是否存在
    def change_password(users, types):
        if session_usertype != "Super" or session_user_id != "31031861754631901758671963372437":
            return jsonify(msg.get("502"), {})
        for i in users:
            so_passwd = des_descrypt(i.get("password"), des_publicKey())
            so_id = i.get("id")
            so_name = i.get("username")
            in_sourcePasswd = base64_decode(old_password)
            in_userid = base64_decode(user_id)
            in_username = base64_decode(username)
            if in_sourcePasswd == so_passwd and in_userid == so_id and in_username == so_name:
                i["password"] = des_encrypt(base64_decode(second_password), des_publicKey())
                # 将新密码写入文件
                if types == "default":
                    file_obj.u = user_list
                else:
                    file_obj.write_black(users)
                # 删除session信息,是修改完密码返回登录页, 如果修改的密码不是当前正在登录的用户,无需返回
                if session.get('username') == in_username:
                    session.pop('username', None)
                    session.pop('usertype', None)
                    session.pop('id', None)
                # 返回成功响应提示
                return jsonify(msg.get("200"))

    if session.get('username') in [item.get("username") for item in user_list]:
        return change_password(user_list, 'default')
    elif session.get('username') in [items.get("username") for items in black_list]:
        return change_password(black_list, 'black')
    else:
        return jsonify(msg.get("502"), {})


# 注销登录
@acs_data.route('/logout', methods=['GET'])
def acs_logout():
    login_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    URL = request.url
    user_ip = request.remote_addr

    username = session.get("username")

    # 获取中英文字典信息
    msg = cookie_get().get("msg", {})
    if not system_time():
        # write log
        write_event_log_file(login_time, "logout", username, user_ip
                             + ', request URL: ' + URL + ', status code: 406  ............ [Failure]')
        return jsonify(msg.get("503", {}))

    try:
        # 将session里面的用户信息删除`
        session.clear()
    except:
        # write log
        write_event_log_file(login_time, "logout", username, user_ip
                             + ', request URL: ' + URL + ', status code: 406  ............ [Failure]')
        return jsonify(code=406, msg=msg.get("logout_error"))

    # write log
    write_event_log_file(login_time, "logout", username, user_ip
                         + ', request URL: ' + URL + ', status code: 200  ............ [Success]')

    return jsonify(code=200, msg=msg.get("logout_ok"))


# 用户管理-获取用户信息列表
@acs_data.route('/user_info_data', methods=["GET"])
@login_requied
@user_authenticate("super")
def user_info_data():
    # 拿取请求参数
    page, page_size = parameter_vilify(request)
    # 实例化文件操作对象, 拿取用户信息数据
    file_obj = FileOperations()
    content = file_obj.u
    for item in content:
        item["password"] = ""
    # 将数据格式化
    cls_obj = FormatConversion(content, page_size, page)
    content = cls_obj.user_group_to_dict()
    return jsonify(content)


# 用户管理-添加用户
@acs_data.route('/add_user', methods=["POST"])
@login_requied
@user_authenticate("super")
def add_user():
    # 拿取中英文词典
    msg = cookie_get().get("msg", {})
    if not system_time():
        return jsonify(msg.get("503", {}))
    # 判断添加用户的操作用户类型是否为super(super才有添加用户的权限)
    if session.get("usertype") != "Super":
        return jsonify(msg.get("502", {}))
    # 拿取请求数据
    request_params = request.json
    # 获取需要添加的用户类型
    usertype = request_params.get("userType")
    # 不予许super类型用户添加
    if usertype == "Super":
        return jsonify(msg.get("504", {}))
    # 获取用户名, 密码, 确认密码, 描述
    username = request_params.get("username")
    pwd = request_params.get("firstPassword")
    pwd2 = request_params.get("secondPassword")
    remark = request_params.get("remark", '')
    menuRole = request_params.get("menuRole", [])
    changetype = request_params.get("changetype", 'kiwict')
    templateStyle = "style/" + request_params.get("templateStyle", '')

    # 获取当前创建时间
    createtime = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

    # 非空判断
    if not all([username, pwd, usertype, pwd2, menuRole]):
        return jsonify(msg.get("402", {}))
    # 判断密码和确认密码是否一致
    if pwd != pwd2:
        return jsonify(code=400, msg=msg.get("password_different"))
    # 将密码base64解码
    password = base64_decode(pwd)
    # 创建文件操作对象, 获取用户信息列表
    file_obj = FileOperations()
    user_list = file_obj.u  # type: list
    user_list1 = user_list.copy()
    user_list1.extend(file_obj.black())
    for user_dict in user_list1:
        # 判断添加的用户名是否已存在
        if user_dict.get("username") == username:
            return jsonify(msg.get("431", {}))
    pwd = des_encrypt(password, des_publicKey())

    # 生成ID
    user_id = generator_user_id_number()
    # 组成添加用户数据对象
    user_info_dict = {"id": user_id, "username": username, "createtime": createtime, "password": pwd,
                      "usertype": usertype,
                      "remark": remark, "changetype": changetype, "uistyle": templateStyle,
                      "stylelist": GetTemplateDirList()}
    # 追加新增用户数据
    try:
        if file_obj.add_user_role_info(user_id, menuRole):
            user_list.append(user_info_dict)
            file_obj.u = user_list
        else:
            return jsonify(msg.get("202", {}))
    except Exception as e:
        return jsonify(msg.get("406", {"msg": "新增用户失败，" + e}))
    return jsonify(msg.get("200", {}))


# 用户管理-修改用户
@acs_data.route('/update_user', methods=["POST"])
@login_requied
@user_authenticate("super")
def update_user_info():
    # 拿取中英文词典
    msg = cookie_get().get("msg", {})
    if not system_time():
        return jsonify(msg.get("503", {}))
    # 判断添加用户的操作用户类型是否为super(super才有添加用户的权限)
    if session.get("usertype") != "Super" or session.get("id") != "31031861754631901758671963372437":
        return jsonify(msg.get("502", {}))
    # 拿取请求数据
    request_params = request.json
    # 获取需要添加的用户类型
    usertype = request_params.get("userType")
    # 获取用户名, 密码, 确认密码, 描述
    id = request_params.get("id")
    username = request_params.get("username")
    remark = request_params.get("remark", '')
    menuRole = request_params.get("menuRole", [])
    changetype = request_params.get("changetype", 'kiwict')
    templateStyle = "style/" + request_params.get("templateStyle", '')

    # 非空判断
    if not all([username, usertype, templateStyle]):
        return jsonify(msg.get("402", {}))

    # 创建文件操作对象, 获取用户信息列表
    file_obj = FileOperations()
    user_list = file_obj.u  # type: list

    # 获取当前创建时间
    createtime = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

    # 组成添加用户数据对象
    user_info_dict = {"id": id, "username": username, "createtime": createtime, "usertype": usertype,
                      "remark": remark, "changetype": changetype, "uistyle": templateStyle,
                      "stylelist": GetTemplateDirList()}

    # 更新用户数据
    try:
        if file_obj.update_user_role_info(user_info_dict.get("id"), menuRole):
            for item in user_list:
                if item.get("id") == user_info_dict.get("id"):
                    item["username"] = user_info_dict.get("username")
                    item["createtime"] = user_info_dict.get("createtime")
                    item["usertype"] = user_info_dict.get("usertype")
                    item["remark"] = user_info_dict.get("remark")
                    item["changetype"] = user_info_dict.get("changetype")
                    item["uistyle"] = user_info_dict.get("uistyle")
                    item["stylelist"] = user_info_dict.get("stylelist")
                    break
            file_obj.u = user_list
        else:
            return jsonify(msg.get("202", {}))
    except Exception as e:
        print(str("更新用户数据异常:") + e)
        return jsonify(msg.get("406", {"error": e}))
    return jsonify(msg.get("200", {}))


# 删除用户
@acs_data.route('/del_user', methods=["POST"])
@login_requied
@user_authenticate("super")
def del_user():
    msg = cookie_get().get("msg", {})
    if not system_time():
        return jsonify(msg.get("503", {}))
    # 判断操作用户是否为super(只允许super类型用户有此权限)
    if session.get("usertype") != "Super":
        return jsonify(msg.get("502", {}))
    # 拿取请求数据
    request_params = request.json
    username = request_params.get("username", "")
    user_id = request_params.get("id")
    login_id = request.cookies["id"]
    # 拿取当前登录用户
    curr_username = session.get("username", "")
    curr_id = session.get("id")
    if curr_username == "":
        return jsonify(msg.get("502", {}))
    # 非空判断
    if username == "":
        return jsonify(msg.get("402", {}))
    # 判断删除的用户当前用户
    if username == curr_username or des_descrypt(login_id, des_publicKey()) != curr_id:
        return jsonify(msg.get("502", {}))

    # 创建FileOperations实例对象; 读取用户信息列表
    file_obj = FileOperations()
    user_list = file_obj.u
    for user in user_list:
        if user.get("username") == username and user_id == user.get("id"):
            # 判断删除用户是否为super, super类型的用户不予许删除
            if (user.get("usertype") == "Super") or (user.get("id") == file_obj.get_super_user_id("admin", "Super")):
                return jsonify(msg.get("502", {}))
            else:
                # 删除对应的用户用权限
                if file_obj.delete_user_role_info(user_id):
                    # 删除用户
                    user_list.remove(user)
                    # 重新写入文件
                    file_obj.u = user_list
                else:
                    return jsonify(msg.get("406", {}))

            break
    return jsonify(msg.get("200", {}))


# 修改参数(setValue)
@acs_data.route('/amend_params', methods=['POST'])
@login_requied
def amend_params_data():
    msg = cookie_get().get("msg", {})
    request_params = request.json
    # 拿取请求数据的参数名和参数值
    parameter_name = request_params.get("parameterName")
    parameter_value = request_params.get("parameterValue")
    parameter_value = html_filter_tags(parameter_value)

    # 模板配置
    matchObj = re.match("style/", parameter_name, 0)
    if matchObj is not None:
        username = request_params.get("username")
        id = request_params["id"]
        createtime = request_params.get("createtime")
        clientname = request.cookies["username"]
        clientid = request.cookies["id"]
        loginname = session.get("username")
        loginid = session.get("id")
        if not all([clientname, clientid, loginname, loginid]):
            return jsonify(msg.get("402", {}))
        if username != loginname or des_descrypt(clientid, des_publicKey()) != loginid:
            return jsonify(msg.get("402", {}))
        userFileClass = FileOperations()
        if userFileClass.update_user_info_with_id(parameter_value, id, username, createtime, loginname):
            resp_msg = msg.get("200", {})
            resp_msg.__setitem__('template_changed', True)
            content = jsonify(resp_msg)
            return content
        else:
            resp_msg = msg.get("500", {})
            resp_msg.__setitem__('template_changed', False)
            connent = jsonify(resp_msg)
            return connent
    else:
        # 参数名非空判断
        if not all([parameter_name]) or parameter_value is None:
            return jsonify(msg.get("402", {}))
        # 组装下发设备数据
        params_list = [{"parameterName": parameter_name, "parameterValue": parameter_value}]
        # 下发到设备
        resp = SetParameterValues(params_list)
        # 判断返回是否成功
        if resp == '' or (resp is not None and resp.get("status") == "1"):
            resp_msg = msg.get("200", {})
        else:
            resp_msg = msg.get("417", {})
        return jsonify(resp_msg)


# 添加对象(addObj)
@acs_data.route('/addObj', methods=['POST'])
@login_requied
def add_object():
    msg = cookie_get().get("msg", {})
    request_data = request.json
    # 拿取添加实例的参数节点
    obj_name = request_data.get("node_name", "")  # type: str
    # 非空判断
    if obj_name == "":
        return jsonify(msg.get("402", {}))
    if not obj_name.endswith('.'):
        obj_name += "."
    # 下发到设备
    resp = get_response_data("AddObject", {"AddObjectName": obj_name})
    # 判断响应数据是否符合格式
    if resp is None or not isinstance(resp, dict):
        return jsonify(msg.get("501", {}))
    # 判断添加实例是否成功
    if resp.get("status") == "1":
        content = msg.get("200", {})
    else:
        content = msg.get("406", {})
    return jsonify(content)


# 删除对象(delObj)
@acs_data.route('/delObj', methods=['POST'])
@login_requied
def del_object():
    msg = cookie_get().get("msg", {})
    request_data = request.json
    request_params = request.args

    if request_params['ids'] == 'ac_file':
        status = True
        del_files = request_data.get('delObj', [])
        for item in del_files:
            status = status and del_ipsec_ac_files('/etc/swanctl/genDir', item.get('filename'))
        if status:
            result = msg.get("200", {})
        else:
            result = msg.get("500", {})
    else:
        # 拿取需要删除的对象
        del_obj = request_data.get("delObj", "")
        if del_obj == "":
            return jsonify(msg.get("402", {}))
        # 下发到设备
        resp = get_response_data("DeleteObject", {"parametersArry": del_obj})
        # 返回数据效验
        if resp is None or not isinstance(resp, dict):
            return jsonify(msg.get("501", {}))
        # 判断删除实例是否成功
        if resp.get("status") == "1":
            result = msg.get("200", {})
        else:
            result = msg.get("406", {})
    return jsonify(result)


# getValue
# 主页(home)
@acs_data.route('/home', methods=['GET'])
@login_requied
def acs_home():
    global amf, ue, content
    msg = cookie_get().get("msg", {})
    content = {"ue": 0, "amf": 0, "upTime": "up 1 week, 6 days, 6 hours, 52 minutes", "opStatus": False,
               "syncStatus": "UNAVAILABLE", "timeStamp": 0}

    # 需要获取的参数列表
    parameters_list = [
        'Device.DeviceInfo.MU.1.Slot.1.UpTime',
        'Device.DeviceInfo.MU.1.Slot.1.ModelName',
        'Device.DeviceInfo.MU.1.Slot.1.SerialNumber',
        'Device.DeviceInfo.MU.1.Slot.1.Manufacturer',
        'Device.DeviceInfo.MU.1.Slot.1.ManufacturerOUI',
        'Device.DeviceInfo.MU.1.Slot.1.FirstUseDate',
        'Device.DeviceInfo.MU.1.Slot.1.HardwareVersion',
        'Device.DeviceInfo.MU.1.Slot.1.SoftwareVersion',
        'Device.DeviceInfo.MU.1.Slot.1.ProvisioningCode',
        'Device.DeviceInfo.MU.1.Slot.1.VendorUnitFamilyType',
        'Device.DeviceInfo.MU.1.Slot.1.DataModel',
        'Device.DeviceInfo.MU.1.Slot.1.DataModelSpecVersion',
        'Device.DeviceInfo.MU.1.Slot.1.ThreeGPPSpecVersion',
        # 'Device.FAP.GPS.LockedLatitude',
        # 'Device.FAP.GPS.LockedLongitude',
    ]

    # 拿取所有的小区实例
    resp = get_response_data('GetFAPServiceInstanceMessage', {})
    if resp is None or not isinstance(resp, dict):
        return jsonify(msg.get("501", {}))
    # 装换数据格式, 让提取数据更方便
    content = tree_data_format(resp, 1)

    # 循环多个小区实例, 并拼接字符串得到该小区的运行状态
    for i in content.get("data"):
        cell_op_state = i.get("id") + ".NR.RAN.OpState"
        # 插入要获取的参数列表中
        parameters_list.insert(0, cell_op_state)
    # 发送GetParameterValues请求拿取参数值
    resp = GetParameterValues(parameters_list)

    # 判断返回是否为空
    if resp is None or not isinstance(resp, dict):
        return jsonify(msg.get("204", {}))

    # 获取参数列表并判断是否符合规定返回
    data_list = resp.get("parametersArry")

    if data_list is None or not isinstance(data_list, list):
        return jsonify(msg.get("501", {}))

    result_list, status, up_time = home_opt_status(data_list)

    # 将返回列表放回返回数据中
    resp["parametersArry"] = result_list
    # 格式化数据返回
    cls_obj = FormatConversion(resp)
    content = cls_obj.resp_to_dict()

    # 发送请求协议获取AMF运行状态和UE数量
    odi_resp = get_response_data("GetODIRequestMessage", {"ueOdiRequest": "UE", "amfOdiRequest": "AMF"})
    # 返回不为None
    if odi_resp is not None and isinstance(odi_resp, dict):
        ue = odi_resp.get("ueOdiResponse", '0')
        amf = odi_resp.get("amfOdiResponse", '0')

    # sync_resp = get_response_data("getSyncInfoMessage", {})
    # sync_resp_values = {"SyncMode": sync_resp.get('parametersArry')[0].get('parameterValue'),
    #                     "SyncStatus": sync_resp.get('parametersArry')[1].get('parameterValue'),
    #                     "GNSSDetail": sync_resp.get('parametersArry')[2].get('parameterValue')}
    # if (sync_resp_values.get("GNSSDetail") == ''):
    #     sync_resp_values["GNSSDetail"] = "0.00, 0.00, 0"

    # 记录全局变量字典(下面的数据都是要实时获取的, 需求是每隔4秒获取一次, 这里定义这个全局字典就是当请求时间间隔小于4秒,
    # 不在发送请求向设备端拿取数据, 且能保证多个浏览器或者多个用户去获取home页面保证4秒内只有一个请求, 节约资源)
    content["ue"] = ue
    content["amf"] = amf
    content["upTime"] = up_time
    content["opStatus"] = status
    # content["syncStatus"] = sync_resp_values
    content["syncStatus"] = {"SyncMode": '', "SyncStatus": '', "GNSSDetail": ''}

    # 记录时间戳, 判断后一次请求是否和前一次请求间隔4秒
    home_data["timeStamp"] = int(time.time())
    home_data["ue"] = ue
    home_data["amf"] = amf
    return jsonify(content)


# 主页(home: 获取实时监测的变化数据,AMF运行状态、ue数量、设备运行时间、小区运行状态)
@acs_data.route('/GetODIRequestMessage', methods=["GET"])
@login_requied
def get_odi_request_message():
    # 获取当前时间戳
    now_time = int(time.time())
    ue = '0'
    amf = '0'
    # 小于四秒返回前一次的全局字典home_data数据
    if now_time - home_data["timeStamp"] < 4:
        pass
    else:
        msg = cookie_get().get("msg", {})
        parameters_list = ['Device.DeviceInfo.MU.1.Slot.1.UpTime']
        # 拿取所有的小区实例
        resp = get_response_data('GetFAPServiceInstanceMessage', {})
        # 装换数据格式, 让提取数据更方便
        content = tree_data_format(resp, 1)
        # 循环多个小区实例, 并拼接字符串得到该小区的运行状态
        for i in content.get("data"):
            cell_op_state = i.get("id") + ".NR.RAN.OpState"
            # 插入要获取的参数列表中
            parameters_list.insert(0, cell_op_state)
        # 发送协议到设备端拿取数据
        resp = GetParameterValues(parameters_list)
        if resp is not None:
            data_list = resp.get("parametersArry")
        else:
            data_list = None
        # 返回数据列表效验
        if data_list is None or not isinstance(data_list, list):
            home_data["timeStamp"] = int(time.time())
            home_data["ue"] = ue
            home_data["amf"] = amf
            return jsonify({"code": 501, "msg": msg.get("501", {}), "data": home_data})

        # 调用方法发送协议拿取首页实时监测的参数数据
        home_opt_status(data_list)

        # 发送协议拿取amf运行状态和UE数量
        odi_resp = get_response_data("GetODIRequestMessage", {"ueOdiRequest": "UE", "amfOdiRequest": "AMF"})
        # 返回数据非空判断
        if odi_resp is not None and isinstance(odi_resp, dict):
            ue = odi_resp.get("ueOdiResponse", '0')
            amf = odi_resp.get("amfOdiResponse", '0')

        # 记录时间戳, 判断后一次请求是否和前一次请求间隔4秒
        home_data["timeStamp"] = int(time.time())
        home_data["ue"] = ue
        home_data["amf"] = amf
    return jsonify({"code": 200, "msg": "OK", "data": home_data})


# 常用参数(generalParameters)->FAPService
@acs_data.route('/generalParameters_FAPService', methods=["GET"])
@login_requied
def general_parameters_fap_service():
    request_params = request.args
    page_size = int(request_params.get("limit", 20))
    page = int(request_params.get("page", 1))
    ids = request_params.get("ids")
    resp = get_response_data("GetRanMessage", {
        "getRanType": "FAPSERVIRCE_COMMON", "getRanInstance": ids + '.'
    })
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 常用参数(generalParameters)->GNB
@acs_data.route('/generalParameters_GNB', methods=["GET"])
@login_requied
def general_parameters_gnb():
    request_params = request.args
    page_size = int(request_params.get("limit", 20))
    page = int(request_params.get("page", 1))
    parameters_list = [
        'Device.Services.FAPService.1.FAPControl.NR.RAN.Common.gNBId',
        'Device.Services.FAPService.1.FAPControl.NR.RAN.Common.gNBIdLength',
        'Device.Services.FAPService.1.FAPControl.NR.RAN.Common.gNBName'
    ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 常用参数(generalParameters)->AMF
@acs_data.route('/generalParameters_AMF', methods=["GET"])
@login_requied
def general_parameters_amf():
    request_params = request.args
    page = int(request_params.get("page", 1))
    page_size = int(request_params.get("limit", 20))
    resp = get_response_data("GetAmfCommonMessage")
    cls_obj = MoreFormatConversion(resp, page, page_size)
    content = cls_obj.data_reconstitution()
    return jsonify(content)


# 同步信息(information) 更多-->弹窗
@acs_data.route('/information_data_popup')
@login_requied
def get_information_popup_data():
    msg = cookie_get().get("msg", {})
    page, page_size, resp_args = parameter_vilify(request, "types", "ids", "filename")
    types = resp_args[0]
    ids = resp_args[1]
    if not all([types, ids]):
        return jsonify(msg.get("402"))
    resp = get_response_data("GetSyncMessage", {
        "getSyncType": "cnmBandInfo",
        "getSyncInfoInstance": ids + "."
    })
    cls_obj = FormatConversion(resp, page_size, page)
    d_content = cls_obj.resp_to_dict()
    return jsonify(d_content)


# 同步信息(information)
@acs_data.route('/information', methods=['GET'])
@login_requied
def acs_device_info():
    alls = request.headers
    print(alls)
    request_params = request.args
    page_size = int(request_params.get("limit", 20))
    page = int(request_params.get("page", 1))
    types = request_params.get('types')

    if types == "0":
        parameters_list = [
            # "Device.DeviceInfo.MU.1.Slot.1.RRU.RruType",
            # "Device.DeviceInfo.MU.1.Slot.1.PackPosition",
            # "Device.DeviceInfo.MU.1.Slot.1.SlotsOccupied",
            # "Device.DeviceInfo.MU.1.UserLabel",
            # "Device.DeviceInfo.MU.1.DnPrefix",
            # "Device.DeviceInfo.MU.1.Slot.1.ManufacturerOUI",
            # "Device.DeviceInfo.MU.1.Slot.1.Manufacturer",
            # "Device.DeviceInfo.MU.1.Slot.1.ModelName",
            # "Device.DeviceInfo.MU.1.Slot.1.SerialNumber",
            # "Device.DeviceInfo.MU.1.Slot.1.UpTime",
            # "Device.DeviceInfo.MU.1.Slot.1.FirstUseDate",
            # "Device.DeviceInfo.MU.1.Slot.1.Status",
            # "Device.DeviceInfo.MU.1.Slot.1.Reboot",
            # "Device.DeviceInfo.MU.1.Slot.1.SwUpgrade.Stage",
            # "Device.DeviceInfo.MU.1.Slot.1.SwUpgrade.FailureCause",
            # "Device.DeviceInfo.MU.1.Slot.1.SwUpgrade.Status",
            # "Device.DeviceInfo.MU.1.",
            # "Device.DeviceInfo.MU.1.SyncOrigin",
            # "Device.FAP.GPS.LockedLongitude",
            # "Device.FAP.GPS.LockedLatitude",
            # "Device.FAP.GPS.NumberOfSatellites"
        ]
        resp1 = get_response_data("GetSyncMessage", {
            "getSyncType": "syncMode", "getSyncInfoInstance": "Device.DeviceInfo.MU.1."
        })
        parametersArray = resp1.get('parametersArry')
        for item in parametersArray:
            if item.get('parameterName') == 'Device.DeviceInfo.MU.1.ClockSynMode':
                value = item.get('parameterValue')
                path = ''
                if value == 'free_run':
                    break
                elif value == 'GPS':
                    path = 'Device.FAP.GPS.'
                elif value == 'CNM':
                    path = 'Device.Services.FAPService.1.CellConfig.1.NMMConfig.'
                if value is not None and value != '':
                    resp2 = get_response_data("GetSyncMessage", {
                        "getSyncType": value, "getSyncInfoInstance": path
                    })
                    if value == 'free_run':
                        resp1 = resp2
                    else:
                        for default in resp2.get('parametersArry'):
                            parametersArray.append(default)
                break

        cls_obj = FormatConversion(resp1, page_size, page)
        content = cls_obj.resp_to_dict()
        return jsonify(content)
    elif types == "1":
        parameters_list = [
            "Device.DeviceInfo.MU.1.Slot.1.HardwareVersion",
            "Device.DeviceInfo.MU.1.Slot.1.SoftwareVersion",
            "Device.DeviceInfo.MU.1.Slot.1.ProvisioningCode",
            "Device.DeviceInfo.MU.1.Slot.1.VendorUnitFamilyType",
            "Device.DeviceInfo.MU.1.Slot.1.DataModel",
            "Device.DeviceInfo.MU.1.Slot.1.DataModelSpecVersion",
            "Device.DeviceInfo.MU.1.Slot.1.ThreeGPPSpecVersion",
        ]
        resp = GetParameterValues(parameters_list)
        cls_obj = FormatConversion(resp, page_size, page)
        content = cls_obj.resp_to_dict()
        return jsonify(content)


# 设备管理(device)->网管参数(HeMS Mgmt)
@acs_data.route('/hm_data', methods=['GET'])
@login_requied
def get_hm_data():
    page, page_size = parameter_vilify(request)
    parameters_list = [
        'Device.ManagementServer.URL',
        'Device.ManagementServer.Username',
        'Device.ManagementServer.Password',
        'Device.ManagementServer.PeriodicInformEnable',
        'Device.ManagementServer.PeriodicInformTime',
        'Device.ManagementServer.PeriodicInformInterval',
        'Device.ManagementServer.ParameterKey',
        'Device.ManagementServer.ConnectionRequestURL',
        'Device.ManagementServer.ConnectionRequestUsername',
        'Device.ManagementServer.ConnectionRequestPassword',
        'Device.ManagementServer.UDPConnectionRequestAddress',
        'Device.ManagementServer.STUNEnable',
        'Device.ManagementServer.STUNServerAddress',
        'Device.ManagementServer.STUNServerPort',
        'Device.ManagementServer.STUNUsername',
        'Device.ManagementServer.STUNPassword',
        'Device.ManagementServer.STUNMaximumKeepAlivePeriod',
        'Device.ManagementServer.STUNMinimumKeepAlivePeriod',
        'Device.ManagementServer.NATDetected'
    ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 设备管理(device)->日志(Log Mgmt)
@acs_data.route('/lm_data')
@login_requied
def get_lm_data():
    page, page_size = parameter_vilify(request)
    parameters_list = [
        'Device.LogMgmt.PeriodicUploadEnable',
        'Device.LogMgmt.URL',
        'Device.LogMgmt.Username',
        'Device.LogMgmt.Password',
        'Device.LogMgmt.PeriodicUploadInterval',
        'Device.LogMgmt.LogLevel',
        'Device.LogMgmt.LimtStorage'
    ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 设备管理(device)->广域网(WAN Mgmt)
@acs_data.route('/wm_data')
@login_requied
def get_wm_data():
    request_params = request.args
    ids = request_params.get("ids")
    page = int(request_params.get("page", 1))
    page_size = int(request_params.get("limit", 20))
    types = request_params.get("types")
    if ids is not None:
        if types == "Ethernet":
            if str(ids.split('.')[len(ids.split('.')) - 1]).__contains__('4'):
                network_type = "WAN_IPADDR_V4"
            elif str(ids.split('.')[len(ids.split('.')) - 1]).__contains__('6'):
                network_type = "WAN_IPADDR_V6"
            elif str(ids.split('.')[len(ids.split('.')) - 1]).__contains__('Dns'):
                network_type = 'WAN_DNS'
            else:
                network_type = ''

            resp = get_response_data("GetWanMessage", {
                "getWanInfoType": network_type,
                "getWanInfoInstance": ids + "."
            })

            if resp != '' and network_type != "WAN_DNS":
                for item in resp.get("parametersArry"):
                    param_name = str(item[0].get("parameterName")).split(".")
                    param_name[len(param_name) - 1] = "Netmask"

                    # 新路径
                    new_param_name = ''
                    for val in param_name:
                        if new_param_name == '':
                            new_param_name = val
                        else:
                            new_param_name = new_param_name + '.' + val

                    if item[0].get("parameterValue") != '':
                        new_param_value = item[0].get("parameterValue").split("/")[1]
                    else:
                        new_param_value = ''

                    netmask = {
                        'parameterName': new_param_name,
                        'parameterRestriction': item[0].get("parameterRestriction"),
                        'parameterType': item[0].get("parameterType"),
                        'parameterValue': new_param_value,
                        'parameterWritable': item[0].get("parameterWritable")
                    }

                    if new_param_value != '':
                        item[0]["parameterValue"] = item[0].get("parameterValue").split("/")[0]

                    item.append(netmask)

        elif types == 'Vlan':
            if str(ids.split('.')[len(ids.split('.')) - 1]).__contains__('4'):
                network_type = "WAN_VLAN_IPADDR_V4"
            elif str(ids.split('.')[len(ids.split('.')) - 1]).__contains__('6'):
                network_type = "WAN_VLAN_IPADDR_V6"
            elif str(ids.split('.')[len(ids.split('.')) - 1]).__contains__('Dns'):
                network_type = 'WAN_VLAN_DNS'
            else:
                network_type = ''
            resp = get_response_data("GetWanMessage", {
                "getWanInfoType": network_type,
                "getWanInfoInstance": ids + "."
            })
            if network_type != "WAN_VLAN_DNS":
                for item in resp.get("parametersArry"):
                    param_name = str(item[0].get("parameterName")).split(".")
                    param_name[len(param_name) - 1] = "Netmask"

                    # 新路径
                    new_param_name = ''
                    for val in param_name:
                        if new_param_name == '':
                            new_param_name = val
                        else:
                            new_param_name = new_param_name + '.' + val

                    if item[0].get("parameterValue") != '':
                        new_param_value = item[0].get("parameterValue").split("/")[1]
                    else:
                        new_param_value = ''

                    netmask = {
                        'parameterName': new_param_name,
                        'parameterRestriction': item[0].get("parameterRestriction"),
                        'parameterType': item[0].get("parameterType"),
                        'parameterValue': new_param_value,
                        'parameterWritable': item[0].get("parameterWritable")
                    }

                    if new_param_value != '':
                        item[0]["parameterValue"] = item[0].get("parameterValue").split("/")[0]

                    item.append(netmask)
        else:
            if str(ids.split('.')[len(ids.split('.')) - 1]).__contains__('4'):
                network_type = "DPAA_IPADDR_V4"
            elif str(ids.split('.')[len(ids.split('.')) - 1]).__contains__('6'):
                network_type = "DPAA_IPADDR_V6"
            elif str(ids.split('.')[len(ids.split('.')) - 1]).__contains__('Dns'):
                network_type = 'DPAA_IPADDR_DNS'
            else:
                network_type = ''

            resp = get_response_data("GetWanMessage", {
                "getWanInfoType": network_type,
                "getWanInfoInstance": ids + "."
            })

            if resp != '' and network_type != "DPAA_IPADDR_DNS":
                for item in resp.get("parametersArry"):
                    param_name = str(item[0].get("parameterName")).split(".")
                    param_name[len(param_name) - 1] = "Netmask"

                    # 新路径
                    new_param_name = ''
                    for val in param_name:
                        if new_param_name == '':
                            new_param_name = val
                        else:
                            new_param_name = new_param_name + '.' + val

                    if item[0].get("parameterValue") != '':
                        new_param_value = item[0].get("parameterValue").split("/")[1]
                    else:
                        new_param_value = ''

                    netmask = {
                        'parameterName': new_param_name,
                        'parameterRestriction': item[0].get("parameterRestriction"),
                        'parameterType': item[0].get("parameterType"),
                        'parameterValue': new_param_value,
                        'parameterWritable': item[0].get("parameterWritable")
                    }

                    if new_param_value != '':
                        item[0]["parameterValue"] = item[0].get("parameterValue").split("/")[0]

                    item.append(netmask)
    else:
        index = int(request_params.get("index", 0))
        if index == 0:
            resp = get_response_data("GetWanMessage", {
                "getWanInfoType": "Ethernet",
                "getWanInfoInstance": "Device.Ethernet.Interface."
            })
        elif index == 1:
            resp = get_response_data("GetWanMessage", {
                "getWanInfoType": "WAN_VLAN",
                "getWanInfoInstance": "Device.Ethernet.VlanInterface."
            })
        else:
            resp = get_response_data("GetWanMessage", {
                "getWanInfoType": "DPAA_ETHERNET",
                "getWanInfoInstance": "Device.Ethernet.DpaaInterface."
            })
    cls_obj = MoreFormatConversion(resp, page, page_size)
    content = cls_obj.data_reconstitution()
    # if types == "Ipv4":
    #     for i in content.get("data", {}):
    #         if i.get("AddressingType", {}) == "dhcp":
    #             for j in i:
    #                 if "permission" in j:
    #                     i[j] = "0"
    return jsonify(content)


# 设备管理(device)->本地路由配置参数管理(IpRoute)
@acs_data.route('/IpRoute', methods=["GET"])
@login_requied
def get_lr_data():
    page, page_size, resp_kwargs = parameter_vilify(request, ids='', types='')
    ids, types = resp_kwargs  # 暂时没有ids
    if types == "拓展":
        instance = ids
        info_type = "route_aaa"
    else:
        instance = "Device.Ethernet.IpRoute."
        info_type = "WAN_IP_ROUTE"
    resp = get_response_data("GetWanMessage", {"getWanInfoType": info_type, "getWanInfoInstance": instance})
    cls_obj = MoreFormatConversion(resp, page, page_size)
    content = cls_obj.data_reconstitution()
    return jsonify(content)


# 设备管理(device)->英特网协议安全(IPsec Mgmt)
@acs_data.route('/im_data')
@login_requied
def get_im_data():
    msg = cookie_get().get("msg", {})
    page, page_size = parameter_vilify(request)
    resp1 = get_response_data("GetIpsecMessage", {
        "getIpsecType": "IPSEC",
        "getIpsecInstance": "Device.IPsec."
    })
    # resp2 = GetParameterValues(['Device.Auth.SerialNumberSec'])
    try:
        resp1.get("parametersArry")
        # .append(resp2.get("parametersArry")[0])
    except:
        return jsonify(msg.get("501"))
    cls_obj = FormatConversion(resp1, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 设备管理(device)->英特网协议安全(IPsec Mgmt)->弹窗
@acs_data.route('/im_data_popup')
@login_requied
def get_im_popup_data():
    msg = cookie_get().get("msg", {})
    page, page_size, resp_args = parameter_vilify(request, "types", "ids", "filename")
    types = resp_args[0]
    ids = resp_args[1]
    filename = resp_args[2]
    d_content = dict()
    if not all([types, ids]):
        return jsonify(msg.get("402"))
    if ids == 'ac_file':
        if types == 'ac_manage':
            file_list = get_ipsec_ac_files('/etc/swanctl/genDir')
            d_content['code'] = 200,
            d_content['msg'] = 'OK',
            d_content['count'] = len(file_list),
            d_content['data'] = file_list
            return jsonify(d_content)
        elif types == 'output':
            d_content = {"code": 200, "msg": "OK", "path": "/etc/swanctl/genDir/" + filename}
            return jsonify(d_content)
    else:
        resp = get_response_data("GetIpsecMessage", {
            "getIpsecType": "IPSEC_" + types,
            "getIpsecInstance": ids + "."
        })
        cls_obj = FormatConversion(resp, page_size, page)
        d_content = cls_obj.resp_to_dict()
        return jsonify(d_content)


# 设备管理(device)->时间服务器(NTP Mgmt)
@acs_data.route('/nm_data')
@login_requied
def get_nm_data():
    page, page_size = parameter_vilify(request)
    parameters_list = [
        'Device.Time.Enable',
        'Device.Time.Port',
        'Device.Time.NTPServer1',
        'Device.Time.NTPServer2',
        'Device.Time.NTPServer3',
        'Device.Time.NTPServer4',
        'Device.Time.NTPServer5',
        # 'Device.Time.CurrentLocalTime',
        # 'Device.Time.LocalTeimeZoneName',
        # "Device.Time.PTPServr",
    ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 设备管理(device)->全球定位系统(GPS)
@acs_data.route('/gps_data')
@login_requied
def get_gps_data():
    page, page_size = parameter_vilify(request)
    parameters_list = [
        'Device.FAP.GPS.LockedLatitude',
        'Device.FAP.GPS.LockedLongitude',
        'Device.FAP.GPS.NumberOfSatellites',
        "Device.DeviceInfo.OpenAccountLocal",
        "Device.DeviceInfo.MU.1.ClockSource",
        "Device.DeviceInfo.MU.1.ClockSyncStatus"
    ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 设备管理(device)->基站标识参数(Gnb)
@acs_data.route('/gnb_data')
@login_requied
def get_gnb_data():
    page, page_size = parameter_vilify(request)
    parameters_list = [
        'Device.Services.FAPService.1.FAPControl.NR.RAN.Common.gNBId',
        'Device.Services.FAPService.1.FAPControl.NR.RAN.Common.gNBIdLength',
        'Device.Services.FAPService.1.FAPControl.NR.RAN.Common.gNBName',
        # 定时器
        'Device.Services.FAPService.1.FAPControl.NR.UeContexRelCmdTimer',
        'Device.Services.FAPService.1.FAPControl.NR.UeContextRelCompTimer',
        'Device.Services.FAPService.1.FAPControl.NR.InitContextSetupTimer',
    ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 设备管理(device)->CA服务器URL(ca)
@acs_data.route('/ca_data')
@login_requied
def get_ca_data():
    page, page_size = parameter_vilify(request)
    parameters_list = [
        # 'Device.CAServer.URL',
        # 'Device.CAServer.Username',
        # 'Device.CAServer.Password'
    ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 设备管理(device)->License
@acs_data.route('/license_data')
@login_requied
def get_license_data():
    page, page_size = parameter_vilify(request)
    resp = get_response_data("LicenseMessage", {
        "LicenseMethod": "getLicenseMsg",
        "LicensePath": "/root/5g.license"
    })
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 设备管理(device)->自动开站(AutoStage)
@acs_data.route('/as_data')
@login_requied
def get_stage_data():
    page, page_size = parameter_vilify(request)
    parameters_list = [
        "Device.Services.FAPService.1.FAPControl.SelfConfig.Startup.Stage",
        "Device.Services.FAPService.1.FAPControl.SelfConfig.Startup.Status",
        "Device.Services.FAPService.1.FAPControl.SelfConfig.Startup.FailureCause",
    ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 设备管理(device)->MU参数配置(EU参数)
@acs_data.route('/mu_data')
@login_requied
def get_mu_data():
    page, page_size = parameter_vilify(request)
    parameters_list = [
        "Device.DeviceInfo.MU.1.EuSyncSource",
        "Device.DeviceInfo.MU.1.EuFrameOffset"
    ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 告警信息(fault)
@acs_data.route('/warn_data', methods=['POST', 'GET'])
@login_requied
def acs_warn_data():
    page, page_size, resp_kwargs = parameter_vilify(request, params="currentAlarm", select='', start='', end='')
    type_params, select, start, end = resp_kwargs
    time_range = start + "~" + end
    resp = GetAlarmMessage([type_params])
    cls_obj = FormatConversion(resp, page_size, page, select=select, time_range=time_range)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 性能管理(PM)
@acs_data.route('/performance_data', methods=["POST", "GET"])
@login_requied
def get_performance_data():
    page, page_size = parameter_vilify(request)
    resp = GetPerformanceMessage("GetPerformanceWarningMessage")
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 性能管理(MR)
@acs_data.route('/MR', methods=["GET"])
@login_requied
def get_mr_data():
    page, page_size = parameter_vilify(request)
    parameters_list = [
        "Device.FAP.MRMgmt.Config.1.UserLabel",
        "Device.FAP.MRMgmt.Config.1.MrEnable",
        "Device.FAP.MRMgmt.Config.1.MrUrl",
        "Device.FAP.MRMgmt.Config.1.MrUsername",
        "Device.FAP.MRMgmt.Config.1.MrPassword",
        "Device.FAP.MRMgmt.Config.1.MeasureType",
        "Device.FAP.MRMgmt.Config.1.OmcName",
        "Device.FAP.MRMgmt.Config.1.MRNCGIList",
        "Device.FAP.MRMgmt.Config.1.SamplePeriod",
        "Device.FAP.MRMgmt.Config.1.UploadPeriod",
        "Device.FAP.MRMgmt.Config.1.SampleBeginTime",
        "Device.FAP.MRMgmt.Config.1.SampleEndTime",
        "Device.FAP.MRMgmt.Config.1.PrbNum",
        "Device.FAP.MRMgmt.Config.1.SubFrameNum",
        "Device.FAP.MRMgmt.Config.1.MeasureItems"
    ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 性能管理(TR)
@acs_data.route('/TR', methods=["GET"])
@login_requied
def get_tr_data():
    page, page_size = parameter_vilify(request)
    parameters_list = [
        'Device.FAP.CallTraceMgmt.Config.1.Enable',
        'Device.FAP.CallTraceMgmt.Config.1.CallTraceUrl',
        'Device.FAP.CallTraceMgmt.Config.1.CallTraceUsername',
        'Device.FAP.CallTraceMgmt.Config.1.CallTracePassword',
        'Device.FAP.CallTraceMgmt.Config.1.CellId',
        'Device.FAP.CallTraceMgmt.Config.1.TraceReference',
        'Device.FAP.CallTraceMgmt.Config.1.InterfacesToTrace',
        'Device.FAP.CallTraceMgmt.Config.1.TraceDepth',
        'Device.FAP.CallTraceMgmt.Config.1.SamplePeriod',
        'Device.FAP.CallTraceMgmt.Config.1.UploadPeriod',

    ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 性能管理(RL)补充采集数据
@acs_data.route('/RL', methods=["GET"])
@login_requied
def get_rl_data():
    page, page_size = parameter_vilify(request)
    parameters_list = [
        'Device.FAP.PerfMgmt.Config.1.ReplenishEnable',
        "Device.FAP.PerfMgmt.Config.1.ReplenishStartTime",
        "Device.FAP.PerfMgmt.Config.1.ReplenishEndTime",
    ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


#
# # 性能管理(SC)软采参数管理
# @acs_data.route('/SC', methods=["GET"])
# @user_authenticate("super")
# def get_sc_data():
#     page, page_size = parameter_vilify(request)
#     parameters_list = [
#         "Device.FAP.SCMgmt.Config.1.SCEnable",
#         "Device.FAP.SCMgmt.Config.1.SCIPAddress",
#         "Device.FAP.SCMgmt.Config.1.SCPort",
#         "Device.FAP.SCMgmt.Config.1.SCLoginID",
#         "Device.FAP.SCMgmt.Config.1.SCPassword",
#         "Device.FAP.SCMgmt.Config.1.SCNCGIList",
#         "Device.FAP.SCMgmt.Config.1.SCGID",
#         "Device.FAP.SCMgmt.Config.1.P2PNEMode",
#         "Device.FAP.SCMgmt.Config.1.SamplePeriod",
#         "Device.FAP.SCMgmt.Config.1.MaxUEs",
#     ]
#     resp = GetParameterValues(parameters_list)
#     cls_obj = FormatConversion(resp, page_size, page)
#     content = cls_obj.resp_to_dict()
#     return jsonify(content)
#
# # 性能管理(Uu-extend)软采参数管理
# @acs_data.route('/Uu-extend', methods=["GET"])
# @user_authenticate("super")
# def get_Uu_extend_data():
#     page, page_size = parameter_vilify(request)
#     parameters_list = [
#         "Device.FAP.SCMgmt.Config.1.UuExtendEnable",
#         "Device.FAP.SCMgmt.Config.1.UuExtendAllEnable",
#         "Device.FAP.SCMgmt.Config.1.UuExtendPHREnable",
#         "Device.FAP.SCMgmt.Config.1.UuExtendULSINREnable",
#         "Device.FAP.SCMgmt.Config.1.UuExtendTAEnable",
#         "Device.FAP.SCMgmt.Config.1.UuExtendCQIEnable",
#         "Device.FAP.SCMgmt.Config.1.UuExtendgNBReceivedInterfereEnable",
#         "Device.FAP.SCMgmt.Config.1.UuExtendDtchPrbAssnUlEnable",
#         "Device.FAP.SCMgmt.Config.1.UuExtendDtchPrbTotUlEnable",
#         "Device.FAP.SCMgmt.Config.1.UuExtendDtchPrbAssnDlEnable",
#         "Device.FAP.SCMgmt.Config.1.UuExtendDtchPrbTotDlEnable",
#     ]
#     resp = GetParameterValues(parameters_list)
#     cls_obj = FormatConversion(resp, page_size, page)
#     content = cls_obj.resp_to_dict()
#     return jsonify(content)

# 性能管理(performance)->性能参数
@acs_data.route('/performance_parameter')
@login_requied
def get_performance_parameter():
    page, page_size = parameter_vilify(request)
    parameters_list = [
        'Device.FAP.PerfMgmt.Config.1.Enable',
        'Device.FAP.PerfMgmt.Config.1.Alias',
        'Device.FAP.PerfMgmt.Config.1.URL',
        'Device.FAP.PerfMgmt.Config.1.Username',
        'Device.FAP.PerfMgmt.Config.1.Password',
        'Device.FAP.PerfMgmt.Config.1.PeriodicUploadInterval',
        'Device.FAP.PerfMgmt.Config.1.PeriodicUploadTime'
        # 'Device.FAP.PerfMgmt.Config.1.BbuCollectionTime',
        # 'Device.FAP.PerfMgmt.Config.1.AauCollectionTime'
    ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 参数配置(config)->AMF
@acs_data.route('/config_afm', methods=['GET'])
@login_requied
def get_config_afm_data():
    page, page_size, resp_kwargs = parameter_vilify(request, ids='', types='')
    ids, types = resp_kwargs
    if types == "GUAMI":
        instance = ids
        info_type = "AMF_GUAMI"
    elif types == "SupportPLMNList":
        instance = ids
        info_type = "AMF_SupportPLMNList"
    elif types == "SliceList":
        instance = ids
        info_type = "AMF_SliceList"
    else:
        instance = "Device.Services.FAPService.1.FAPControl.NR.AMFPoolConfigParam."
        info_type = "AMF"
    resp = get_response_data("GetAmfMessage", {"getAmfInfoType": info_type, "getAmfInfoInstance": instance})
    cls_obj = MoreFormatConversion(resp, page, page_size)
    content = cls_obj.data_reconstitution()
    return jsonify(content)


# 参数配置(config)->流控制传输协议(sctp)
@acs_data.route('/config_sctp', methods=['GET'])
@login_requied
def get_config_sctp_data():
    page, page_size = parameter_vilify(request)
    resp = get_response_data("GetSctpMessage", {})
    cls_obj = MoreFormatConversion(resp, page, page_size)
    content = cls_obj.data_reconstitution()
    return jsonify(content)


# 参数配置(config)->Xn配置(neighStation)
@acs_data.route('/config_NeighStation', methods=['GET'])
@login_requied
def get_neigh_station():
    page, page_size, resp_args = parameter_vilify(request, 'TA', 'PL', 'types')
    TA_id, PLMN_id, types = resp_args
    if types == "X2":
        parameters_list = [
            'Device.Services.FAPService.1.FAPControl.NR.X2EndcPrepTimer',
            'Device.Services.FAPService.1.FAPControl.NR.X2EndcOverallTimer',
            'Device.Services.FAPService.1.FAPControl.NR.X2RetransmitTimer',
            'Device.Services.FAPService.1.FAPControl.NR.NumX2MsgRetries',
            'Device.Services.FAPService.1.FAPControl.NR.X2.LocalPort'
        ]
        resp = GetParameterValues(parameters_list)
        cls_obj = FormatConversion(resp, page_size, page)
        content = cls_obj.resp_to_dict()
    elif types == "Xn":
        parameters_list = [
            'Device.Services.FAPService.1.FAPControl.NR.XnRelocPrepTimer',
            'Device.Services.FAPService.1.FAPControl.NR.XnRelocOverallTimer',
            'Device.Services.FAPService.1.FAPControl.NR.XnRetransmitTimer',
            'Device.Services.FAPService.1.FAPControl.NR.NumXnMsgRetries',
            'Device.Services.FAPService.1.FAPControl.NR.Xn.LocalPort',
            'Device.Services.FAPService.1.FAPControl.NR.Xn.LocalServerIp'
        ]
        resp = GetParameterValues(parameters_list)
        cls_obj = FormatConversion(resp, page_size, page)
        content = cls_obj.resp_to_dict()
    else:
        if types == "NbrEnbInfo":
            resp = get_response_data("GetNeighStationMessage", {
                "getNeighStationType": "EnbStation",
                "neighStationMessage": "Device.Services.FAPService.1.FAPControl.NR.X2IpAddrMapInfo."
            })
        elif types == "NbrGnbInfo":
            resp = get_response_data("GetNeighStationMessage", {
                "getNeighStationType": "firstFloor",
                "neighStationMessage": "Device.Services.FAPService.1.FAPControl.NR.XnIpAddrMapInfo"
            })
        elif TA_id is not None:
            resp = get_response_data(
                "GetNeighStationMessage",
                {
                    "getNeighStationType": "secondFloor",
                    "neighStationMessage": TA_id
                }
            )
        elif PLMN_id is not None:
            resp = get_response_data(
                "GetNeighStationMessage",
                {
                    "getNeighStationType": "thirdFloor",
                    "neighStationMessage": PLMN_id
                }
            )
        else:
            resp = get_response_data(
                "GetNeighStationMessage",
                {
                    "getNeighStationType": "firstFloor",
                    "neighStationMessage": "Device.Services.FAPService.1.FAPControl.NR."
                }
            )
        cls_obj = MoreFormatConversion(resp, page, page_size)
        content = cls_obj.data_reconstitution()

    return jsonify(content)


# 获取树状结构数据
@acs_data.route('/config_serviceTree', methods=['GET'])
@login_requied
def config_service_tree():
    request_params = request.args
    types = request_params.get("types", '1')  # type: str

    if types.isdigit():
        types = int(request_params.get("types", '1'))
    if types == 7:
        if VersionCheck():
            resp = get_response_data("GetEuInfoMessage", {
                "getEuInfoType": "EuInstance",
                "getEuInfoInstance": "Device.DeviceInfo.EU."
            })
        else:
            EU_Enable = GetEUFromWhichWay()
            if EU_Enable:
                resp = GenerateEUListFromCustom()
            else:
                resp = get_response_data("GetEuInfoMessage", {
                    "getEuInfoType": "EuInstance",
                    "getEuInfoInstance": "Device.DeviceInfo.EU."
                })
    else:
        resp = get_response_data('GetFAPServiceInstanceMessage', {})

    content = tree_data_format(resp, types)
    return jsonify(content)


# 参数配置(config)->无线配置(service)->总览(overall)
@acs_data.route('/config_serviceOverall', methods=['GET'])
@login_requied
def get_service_overall_data():
    request_params = request.args
    ids = request_params.get("id")
    types = request_params.get("type")
    page_size = int(request_params.get("limit", 20))
    page = int(request_params.get("page", 1))
    option_dict = {
        "State": "getOverAllState",
        # "SecGw": "getOverAllSecGw",
        "CAC": "getOverAllCac",
        "SecurityAlgorithm": "getSecurityFirstFloor",
        "VoNR": "VONR",
        "5GC": "get5GCPara",
        "Broadcast": "OVER_ALL_BROADCAST_PARA_MANAGE",
        "0": "getOverAllCellContrl",
        "1": "getOverAllCellPara",
        "LoadBalancing": "getOverAllLoadBalance",
        "EnergySaveParam": "OVER_ALL_EnergySaveParam",
        "NoiseAndRouteIdx": "OVER_ALL_NoiseAndRouteIdx",
        "RadioBearParam": "OVER_ALL_RadioBearParam",
        "PowerConsume": "getBBUPowerSwitch"
    }
    option = option_dict.get(types, '')
    msg = cookie_get().get("msg", {})

    if option == "":
        return jsonify(msg.get("400", {}))

    if types == "VoNR":
        resp = get_response_data("GetVonrMessage", {"getVonrType": option, "VonrMessage": ids + '.'})
        # cls_obj = MoreFormatConversion(resp, page, page_size)
        # content = cls_obj.data_reconstitution()
        # return jsonify(content)
    elif types == "RadioBearParam":
        page, page_size, resp_kwargs = parameter_vilify(request, ids='', types='')
        resp = get_response_data("GetOverAllMessage", {
            "GetOverAllType": option,
            "overAllMessage": ids + ".NR.RadioBearParam."
        })
        cls_obj = MoreFormatConversion(resp, page, page_size)
        content = cls_obj.data_reconstitution()
        return jsonify(content)
    elif types == "PowerConsume":
        page, page_size, resp_kwargs = parameter_vilify(request, ids='', types='')
        resp = get_response_data("GetOverAllMessage", {
            "GetOverAllType": option,
            "overAllMessage": ids + "."
        })

        PowerConsumeDataList = get_PowersumeDataFromFile("/opt/jwinx/oam/Log", "BBUpower_consum.log")

        if len(resp) <= 0:
            resp = dict()
            switchParam = list()
            paramDict = dict()
            paramDict["parametersName"] = ""
            paramDict["parametersValue"] = "0"
            switchParam.append(paramDict)
            resp["PowerConSumeSwitchStatus"] = switchParam
            if PowerConsumeDataList is not None:
                resp["parametersArry"] = PowerConsumeDataList
            else:
                resp["parametersArry"] = []
        else:
            # 读文件 [/opt/jwinx/oam/Log/BBUpower_consum.log]
            if resp.get("parametersArry") is not None:
                resp["PowerConSumeSwitchStatus"] = list((resp.get("parametersArry"))).copy()
                (resp.get("parametersArry")).clear()
                if PowerConsumeDataList is not None:
                    resp.get("parametersArry").extend(PowerConsumeDataList)
                else:
                    resp.get("parametersArry").extend([])

        cls_obj = FormatConversion(resp, page_size, page)
        content = cls_obj.resp_to_dict()
        return jsonify(content)
    else:
        resp = get_response_data("GetOverAllMessage", {
            "GetOverAllType": option,
            "overAllMessage": ids + "."
        })

    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 参数配置(config)->无线配置(service)->总览(overal)->弹窗
@acs_data.route('/config_serviceOverall_pop', methods=['GET'])
@login_requied
def get_service_overall_data_pop():
    request_params = request.args
    page = int(request_params.get("page", 1))
    page_size = int(request_params.get("limit", 20))
    ids = request_params.get("id")
    pop_type = request_params.get("pop_type")
    if pop_type == "Integrity":
        resp = get_overall_message("getSecurityList", ids)
    elif pop_type == "Cipher":
        resp = get_overall_message("getSecurityList", ids)
    elif pop_type == "VoNRList":
        resp = get_response_data("GetVonrMessage", {"getVonrType": "OVER_ALL_VONR_List", "VonrMessage": ids})
    elif pop_type == "PdcpInitParam":
        resp = get_response_data("GetVonrMessage", {"getVonrType": "OVER_ALL_VoNR_PdcpInitParam", "VonrMessage": ids})
    elif pop_type == "PLMNList":
        resp = get_response_data("GetOverAllMessage",
                                 {"getOverAllType": "OVER_ALL_BROADCAST_PARA_MANAGE_PLMN", "overAllMessage": ids})
    elif pop_type == "IdleModeRATMobile":
        resp = get_response_data("GetOverAllMessage",
                                 {"getOverAllType": "OVER_ALL_BROADCAST_PARA_MANAGE_SIB5", "overAllMessage": ids})
    else:
        type_id = request_params.get("type_id")
        if type_id == "2":
            resp = get_overall_message("getSliceList", ids + '.SliceList.')
        elif type_id == "1":
            resp = get_overall_message("getPlmnList", ids + '.PLMNList.')
        else:
            resp = get_overall_message("getTaList", ids)
    cls_obj = MoreFormatConversion(resp, page, page_size)
    content = cls_obj.data_reconstitution()
    return jsonify(content)


# 参数配置(config)->无线配置(service)->无线接入网(ran)
@acs_data.route('/ran', methods=['GET'])
@login_requied
def get_service_ran_data():
    request_params = request.args
    ids = request_params.get("ids")
    types = request_params.get("types")
    page_size = int(request_params.get("limit", 20))
    page = int(request_params.get("page", 1))
    option_dict = {
        # 物理层管理参数
        "RACH": "RACH",
        "RAN_PHY_BWP": "RAN_PHY_BWP",
        "SSB": "SSB",
        "PUCCH": "PUCCH",
        "PDCCH": "firstPhyPdcch",
        "PUSCH": "PUSCH",
        "PDSCH": "PDSCH",
        "TddULDL": "TddULDL",
        "FrequencyInfo": "FrequencyInfo",
        "PhyUl": "RAN_PHY_PHYU1",
        "RAN_PHY_CSI": "RAN_PHY_CSI",
        "Mac": "MAC",
        "RrcTimers": "RRC",
        "Ocng": "OCNG",
        "AdmissionControl": "RAN_ADMISSION_CONTROL",
        "Mib": "RAN_MIB",
        "Sib": "RAN_SIB1",
        "NoiseStatus_Rb": "RAN_NoiseStatus_Rb",
        "Antenna": "Antenna",
    }
    msg = cookie_get().get("msg")
    option = option_dict.get(types, '')
    if option == '':
        return jsonify(msg.get("400", {}))
    if types == "NoiseStatus_Rb":  # 底噪图表
        resp = get_response_data("GetPhySecondFloorMessage", {
            "getPhyType": option,
            "getPhyInstance": ids + ".NR.RAN.NoiseStatus.Rb."
        })
        cls_obj = MoreFormatConversion(resp, page, page_size)
        content = cls_obj.data_no_paging()
        return jsonify(content)
    elif types == "PDCCH":  # PDCCH特殊请求键值
        resp = get_response_data("GetPhyPdcchMessage", {
            "getPhyPdcchType": option,
            "getPhyPdcchInstance": ids + "."
        })
    else:
        # 正常4组参数显示
        resp = get_response_data("GetRanMessage", {
            "getRanType": option,
            "getRanInstance": ids + '.'
        })

    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 获取底噪信息同时将Cell ID提交到EU
@acs_data.route('/ran_NoiseAverageSet', methods=['GET'])
@login_requied
def set_service_ran_noiseaverage():
    request_params = request.args
    ids = request_params.get("ids")
    types = request_params.get("types")
    msg = cookie_get().get("msg")
    page_size = int(request_params.get("limit", 20))
    page = int(request_params.get("page", 1))

    cell_id = ''
    if types == '' or ids == '':
        return jsonify(msg.get("400", {}))

    fullpath = ids.split(".", -1)
    i = 0
    for item in fullpath:
        if item == "CellConfig":
            cell_id = fullpath[i + 1]
            break
        i = i + 1

    resp = get_response_data("SetParameterValues", {"parametersArry": [
        {"parameterName": "Device.DeviceInfo.MU.1.Slot.1.EU.1.NoiseStatus", "parameterValue": cell_id}]})
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(resp)


# 查询底噪信息平均值
@acs_data.route('/ran_NoiseAverageGet', methods=['GET'])
@login_requied
def get_service_ran_noiseaverage():
    request_params = request.args
    ids = request_params.get("ids")
    types = request_params.get("types")
    msg = cookie_get().get("msg")
    page_size = int(request_params.get("limit", 20))
    page = int(request_params.get("page", 1))

    cell_id = ''
    if types == '' or ids == '':
        return jsonify(msg.get("400", {}))

    fullpath = ids.split(".", -1)
    i = 0
    for item in fullpath:
        if item == "CellConfig":
            cell_id = fullpath[i + 1]
            break
        i = i + 1

    parameters_list = ['Device.Services.FAPService.1.CellConfig.' + cell_id + '.NR.Capabilities.NoisePwdBm']
    resp = GetParameterValues(parameters_list)

    # 判断返回是否为空
    if resp is None or not isinstance(resp, dict):
        return jsonify(msg.get("204", {}))
    data_list = resp.get("parametersArry")
    if data_list is None or not isinstance(data_list, list):
        return jsonify(msg.get("501", {}))

    # 格式化数据返回
    cls_obj = FormatConversion(resp)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


@acs_data.route('/rans', methods=['GET'])
@login_requied
def get_service_rans_data():
    page, page_size, (ids, types) = parameter_vilify(request, "ids", "types")
    if types == "Antenna":
        parameters_list = [
            ids + ".NR.RAN.PHY.Antenna.NumOfTxAntenna",
            ids + ".NR.RAN.PHY.Antenna.NumOfRxAntenna"
        ]
    elif types == "BWPDLS":
        parameters_list = [
            ids + ".NR.RAN.PHY.BWPDL.StartPrbPosition",
            ids + ".NR.RAN.PHY.BWPDL.Bandwidth",
            ids + ".NR.RAN.PHY.BWPDL.SubcarrierSpacing",
            ids + ".NR.RAN.PHY.BWPDL.CyclicPrefix"
        ]
    else:
        parameters_list = [
            ids + ".NR.RAN.PHY.BWPUL.StartPrbPosition",
            ids + ".NR.RAN.PHY.BWPUL.Bandwidth",
            ids + ".NR.RAN.PHY.BWPUL.SubcarrierSpacing",
            ids + ".NR.RAN.PHY.BWPUL.CyclicPrefix",
            ids + ".NR.RAN.PHY.BWPUL.TimeAlignmentTimerCommon"
        ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 参数配置(config)->无线配置(service)->无线接入网(ran)->BWPDL
@acs_data.route('/ran_BWPDL', methods=['GET'])
@login_requied
def get_service_bwpdl_data():
    request_params = request.args
    ids = request_params.get("ids")
    types = request_params.get("types")
    page_size = int(request_params.get("limit", 20))
    page = int(request_params.get("page", 1))
    option_dict = {
        "BD": "BWP_DL",
        "BDP": "BWP_DL_PCCH",
        "BDPCR": "BWP_DL_PDCCH_CommonControlResourceSet",
        "BDPFS": "BWP_DL_PDCCH_First_SearchSpaceList",
        "BDPCC": "BWP_DL_PDCCH_ConfigCommon",
        "BDFP": "BWP_DL_First_PDSCH",
    }
    msg = cookie_get().get("msg")
    option = option_dict.get(types, '')
    if option == '':
        return jsonify(msg.get("400", {}))

    else:
        resp = get_response_data("GetBwpMessage", {
            "getBwpType": option,
            "getBwpInstance": ids
        })
    cls_obj = MoreFormatConversion(resp, page, page_size)
    content = cls_obj.data_reconstitution()
    return jsonify(content)


# 参数配置(config)->无线配置(service)->无线接入网(ran)->BWPUL
@acs_data.route('/ran_BWPUL', methods=['GET'])
@login_requied
def get_service_bwpul_data():
    request_params = request.args
    ids = request_params.get("ids")
    types = request_params.get("types")
    page_size = int(request_params.get("limit", 20))
    page = int(request_params.get("page", 1))
    option_dict = {
        "BU": "BWP_UL",
        "BUP": "BWP_UL_PUCCH",
        "BUPS": "BWP_UL_PUSCH",
        "BUPFA": "BWP_UL_PUSCH_First_AllocationList",
        "BUR": "BWP_UL_RACHConfigCommon",
        "BURC": "BWP_UL_RACHConfigGeneric",
    }
    msg = cookie_get().get("msg")
    option = option_dict.get(types, '')
    if option == '':
        return jsonify(msg.get("400", {}))

    else:
        resp = get_response_data("GetBwpMessage", {
            "getBwpType": option,
            "getBwpInstance": ids
        })
    cls_obj = MoreFormatConversion(resp, page, page_size)
    content = cls_obj.data_reconstitution()
    return jsonify(content)


# 参数配置(config)->无线配置(service)->无线接入网(ran)->BWPDL/UL/DRX_pop
@acs_data.route('/ran_BWP_pop', methods=['GET'])
@login_requied
def get_service_bwp_pop_data():
    request_params = request.args
    ids = request_params.get("ids")
    types = request_params.get("types")
    page_size = int(request_params.get("limit", 20))
    page = int(request_params.get("page", 1))
    if not ids.endswith('.'):
        ids += "."
    option_dict = {
        "CommonSearchSpaceList": "BWP_DL_PDCCH_CommonSearchSpaceList",
        "PDSCHTimeDomainResourceAllocationList": "BWP_DL_PDSCH",
        "PUSCHTimeDomainAllocationList": "BWP_UL_PUSCH_TimeDomainAllocationList",
        "PLMNList": "DRX_PLMNList",
    }
    msg = cookie_get().get("msg")
    option = option_dict.get(types, '')
    if option == '':
        return jsonify(msg.get("400", {}))

    else:
        resp = get_response_data("GetBwpMessage", {
            "getBwpType": option,
            "getBwpInstance": ids
        })
    cls_obj = MoreFormatConversion(resp, page, page_size)
    content = cls_obj.data_reconstitution()
    return jsonify(content)


# 参数配置(config)->无线配置(service)->无线接入网(ran)->DRX
@acs_data.route('/ran_DRX', methods=['GET'])
@login_requied
def get_service_drx_data():
    request_params = request.args
    ids = request_params.get("ids")
    types = request_params.get("types")
    page_size = int(request_params.get("limit", 20))
    page = int(request_params.get("page", 1))
    option_dict = {
        "DRX": "DRX",
    }
    msg = cookie_get().get("msg")
    option = option_dict.get(types, '')
    if option == '':
        return jsonify(msg.get("400", {}))

    else:
        resp = get_response_data("GetBwpMessage", {
            "getBwpType": option,
            "getBwpInstance": ids
        })
    cls_obj = MoreFormatConversion(resp, page, page_size)
    content = cls_obj.data_reconstitution()
    return jsonify(content)


# 参数配置(config)->无线配置(service)->无线接入网(ran)->弹窗
@acs_data.route('/ran-popup', methods=['GET'])
@login_requied
def get_service_ran_pop_data():
    page, page_size, resp_args = parameter_vilify(request, 'ids', 'types')
    ids, types = resp_args
    if types == "PDSCH":
        request_dict = {
            "getPhyType": "PdschList",
            "getPhyInstance": ids + '.',
        }
    elif types == "PUSCH":
        request_dict = {
            "getPhyType": "PuschList",
            "getPhyInstance": ids + '.',
        }
    elif types == "PDCCH" or types == "BWPUL" or types == "BWPDL":
        if types == "PDCCH":
            request_dict = {
                "getPhyPdcchType": "secondPhyPdcch",
                "getPhyPdcchInstance": ids + '.',
            }
            resp = get_response_data("GetPhyPdcchMessage", request_dict)
        else:
            request_dict = {
                "getRanType": types,
                "getRanInstance": ids + '.',
            }
            resp = get_response_data("GetRanMessage", request_dict)
        cls_obj = FormatConversion(resp, page_size, page)
        content = cls_obj.resp_to_dict()
        return jsonify(content)
    else:
        request_dict = {
            "getPhyType": types,
            "getPhyInstance": ids + '.',
        }
    resp = get_response_data("GetPhySecondFloorMessage", request_dict)
    cls_obj = MoreFormatConversion(resp, page, page_size)
    content = cls_obj.data_reconstitution()
    return jsonify(content)


# 参数配置(config)->无线配置(service)->相邻小区(neighCell)
@acs_data.route('/config_serviceNeighCell', methods=['GET'])
@login_requied
def get_service_neigh_cell_data():
    page, page_size, resp_args, resp_kwargs = parameter_vilify(request, 'ids', types="NR")
    ids = resp_args[0]
    types = str(resp_kwargs[0]).split('?')[0]
    option_dict = {
        "NR": "NR",
        "LTE": "LTE",
        "SON": "SON",
        "ANR_CONFIG_FILE_DOWNLOAD": "ANR_CONFIG_FILE_DOWNLOAD"
    }
    option = option_dict.get(types, '')
    msg = cookie_get().get("msg")
    if option == '':
        return jsonify(msg.get("400", {}))
    elif option == "SON":
        resp = get_son_data("/opt/jwinx/oam/Log/", "anrlog.txt")
        cls_obj = MoreFormatConversion(resp, page, page_size)
        content = cls_obj.data_reconstitution()
        return jsonify(content)
    elif option == "ANR_CONFIG_FILE_DOWNLOAD":
        param = request.args
        fileName = param.get("filename")
        filePath = '/opt/jwinx/oam/Log/'
        resp = make_response(send_from_directory(filePath, fileName, as_attachment=True))
        # atype = param.get("types")
        # if atype not in ["log", "identity"]:
        #    get_response_data("GetDownloadRspMessage", {"DownloadStatus": "DownLoadSucess", "DownloadFileName": fileName})
        return resp
    else:
        resp = get_response_data('GetNeighCellsMessage', {
            "getNeighCellsType": types,
            "neighCellsMessage": ids + '.'
        })
        cls_obj = MoreFormatConversion(resp, page, page_size)
        content = cls_obj.data_reconstitution()
        return jsonify(content)


# 参数配置(config)->无线配置(service)->移动性管理(mobility)
@acs_data.route('/config_serviceMobility', methods=['GET'])
@login_requied
def get_service_mobility():
    page, page_size, resp_args = parameter_vilify(request, 'ids', 'types')
    ids, types = resp_args
    if types == "HO-Common":
        request_dict = {
            "getMobilityType": "HoCommon",
            "getMobilityInstance": ids + '.'
        }
    elif types == "Intra-freq":
        request_dict = {
            "getMobilityType": "INTRA_FREQ_RESELECTION",
            "getMobilityInstance": ids + '.'
        }
        # resp = get_response_data("GetMobilityMessage", request_dict)
        # cls_obj = FormatConversion(resp, page_size, page)
        # content = cls_obj.resp_to_dict()
        # for i in content.get("data"):
        #     if i.get("parameterValue") == "more":
        #         content.get("data").remove(i)
        # return jsonify(content)
    elif types == "Reselection-Common":
        request_dict = {
            "getMobilityType": "ReselectionCommon",
            "getMobilityInstance": ids + '.'
        }
    elif types == "Inter-rat":
        request_dict = {
            # "getMobilityType": "INTER_RAT_RESELECTION",
            "getMobilityType": "INTER_RAT_RESELECTION_EUTRA",
            "getMobilityInstance": ids + '.'
        }
    elif types == "InactiveMode":
        request_dict = {
            "getMobilityType": "MOBILITY_InactiveMode",
            "getMobilityInstance": ids + '.'
        }
    elif types == "Timer":
        request_dict = {
            "getMobilityType": "MOBILITY_Timer",
            "getMobilityInstance": ids + '.'
        }
    elif types == "IntraFreqMeas":
        request_dict = {
            "getMobilityType": "MOBILITY_IntraFreq_MeasureCtrl",
            "getMobilityInstance": ids + '.'
        }
    else:
        if types == "A1":
            request_dict = {
                "getMobilityType": "A1",
                "getMobilityInstance": ids + '.'
            }
        elif types == "A2":
            request_dict = {
                "getMobilityType": "A2",
                "getMobilityInstance": ids + '.'
            }
        elif types == "A3":
            request_dict = {
                "getMobilityType": "A3",
                "getMobilityInstance": ids + '.'
            }
        elif types == "A4":
            request_dict = {
                "getMobilityType": "A4",
                "getMobilityInstance": ids + '.'
            }
        elif types == "A5":
            request_dict = {
                "getMobilityType": "A5",
                "getMobilityInstance": ids + '.'
            }
        elif types == "A6":
            request_dict = {
                "getMobilityType": "A6",
                "getMobilityInstance": ids + '.'
            }
        elif types == "B1":
            request_dict = {
                "getMobilityType": "B1",
                "getMobilityInstance": ids + '.'
            }
        elif types == "B2":
            request_dict = {
                "getMobilityType": "B2",
                "getMobilityInstance": ids + '.'
            }
        elif types == "Periodicity":
            request_dict = {
                "getMobilityType": "Periodicity",
                "getMobilityInstance": ids + '.'
            }
        elif types == "InterFreqMeas":
            request_dict = {
                "getMobilityType": "MOBILITY_InterFreq_MeasureCtrl_SSBCarrier",
                "getMobilityInstance": ids + '.'
            }
            resp = get_response_data("GetMobilityMessage", request_dict)
            cls_obj = MoreFormatConversion(resp, page, page_size)
            content = cls_obj.data_reconstitution(types="two")
            return jsonify(content)
        elif types == "InterRat-EutraMeasObj":
            request_dict = {
                "getMobilityType": "MOBILITY_INTER_RAT_EutraMeasObj",
                "getMobilityInstance": ids + '.'
            }
        elif types == "CGI":
            request_dict = {
                "getMobilityType": "ReportCGI",
                "getMobilityInstance": ids + '.'
            }
        else:
            request_dict = {
                "getMobilityType": "INTER_FREQ_RESELECTION",
                "getMobilityInstance": ids + '.'
            }

        resp = get_response_data("GetMobilityMessage", request_dict)
        cls_obj = MoreFormatConversion(resp, page, page_size)
        content = cls_obj.data_reconstitution()
        return jsonify(content)
    resp = get_response_data("GetMobilityMessage", request_dict)
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 参数配置(config)->无线配置(service)->移动性管理(mobility)->弹窗
@acs_data.route('/config_serviceMobility_pop', methods=['GET'])
@login_requied
def get_mobility_pop():
    msg = cookie_get().get("msg", {})
    page, page_size, resp_args = parameter_vilify(request, 'types', 'p_name')
    types, paramName = resp_args
    if not paramName.endswith('.'):
        paramName += "."
    if not all([types, paramName]):
        return jsonify(msg.get("402"))

    options = {
        "PLMN": {
            "getMobilityType": "MOBILITY_PlmnList_PlmnId",
            "getMobilityInstance": paramName
        },
        "MultiFrequencyBandListNRSib": {
            "getMobilityType": "MOBILITY_IntraFreq_MultiFrequencyBandListNRSib",
            "getMobilityInstance": paramName
        },
        "INTER_RAT_RESELECTION_EUTRAFreq": {
            "getMobilityType": "INTER_RAT_RESELECTION_EUTRAFreq",
            "getMobilityInstance": paramName
        },
        "Mobility_HoCommon_QuantityConfigNR": {
            "getMobilityType": "Mobility_HoCommon_QuantityConfigNR",
            "getMobilityInstance": paramName
        },
        "CellReselectionPrioritySpecial": {
            "getMobilityType": "MOBILITY_CellReselectionPrioritySpecial",
            "getMobilityInstance": paramName
        },
        "MOBILITY_IntraFreq_NrNsPmax": {
            "getMobilityType": "MOBILITY_IntraFreq_NrNsPmax",
            "getMobilityInstance": paramName
        }
    }

    request_dict = options.get(types, {"getDeepMobilityType": "four", "getDeepMobilityInstance": paramName})
    resp = get_response_data("GetMobilityMessage", request_dict)
    identifying = "one"
    if types == "Mobility_HoCommon_QuantityConfigNR":
        identifying = "two"
    cls_obj = MoreFormatConversion(resp, page, page_size)
    content = cls_obj.data_reconstitution(identifying)
    return jsonify(content)


# 参数配置(config)->无线配置(service)->自组网(son)
@acs_data.route('/config_serviceSon', methods=['GET'])
@login_requied
def get_service_son_data():
    page, page_size, resp_args = parameter_vilify(request, 'ids', 'types')
    parameters_list = [
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.SONSysMode",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.SONWorkMode",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.PCIOptEnable",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.PCIReconfigWaitTime",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.PCI.Enable",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.CandidateARFCNList",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.CandidatePCIList",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.ANR.Enable",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.ANR.InterFeqEnable",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.ANR.InterFeqCandidateARFCNList",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.ANR.EUTRANEnable",

        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.ANR.BiNRCellEnable",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.ANR.RSRPReferenceEnable",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.ANR.A3RSRPOffset",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.ANR.RSRPThreshold",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.ANR.KPIPeriod",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.ANR.AutoAdjustEnable",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.ANR.BlackList.NoHOThreshold",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.ANR.BlackList.HOSuccessThreshold",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.ANR.AutoRemoveEnable",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.ANR.AutoRemovePeriod",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.ANR.AutoRemoveMaxCell",

        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.ARFCNEnable",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.MaxNRNeighbourCellNum",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.MaxLTENeighbourCellNum",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.ReSynCellEnable",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.PowerEnable",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.NRSnifferFreqBandList",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.NRSnifferChannelList",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.LTESnifferFreqBandList",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.LTESnifferChannelList",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.MROEnable",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.SHEnable",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.SyncMode",
        "Device.Services.FAPService.1.FAPControl.NR.SelfConfig.SONConfigParam.PCIFoundTime",
    ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 参数配置(config)->无线配置(service)->Qos(dataModel以前版和修改版都包括再里面)
@acs_data.route('/config_serviceQos', methods=['GET'])
@login_requied
def get_service_qos_data():
    page, page_size, resp_args = parameter_vilify(request, 'ids', 'Qos_type')
    ids, types = resp_args
    if types == "NSAQos1":
        get_qos_type = "NSA_FIRST_QOS"
    elif types == "NSAQos2":
        get_qos_type = "NSA_SECOND_QOS"
    elif types == "SAQos1":
        get_qos_type = "SA_FIRST_QOS"
    elif types == "SAQos2":
        get_qos_type = "SA_SECOND_QOS"
    elif types == "SAQosSlicelist":
        get_qos_type = "SA_SLICELIST_QOS"
    elif types == "SAQosFloatList":
        get_qos_type = "SA_FLOATLIST_QOS"
    elif types == "SAQosPLMNList":
        get_qos_type = "SA_PLMN_QOS"
    else:
        get_qos_type = "SA_FIRST_QOS"

    resp = get_response_data("GetQosMessage", {"getQosType": get_qos_type, "getQosInstance": ids + "."})
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 参数配置(config)->EU
@acs_data.route('/config_EU', methods=['GET'])
@login_requied
def get_config_eu_data():
    msg = cookie_get().get("msg", {})
    global parameterValue
    page, page_size, resp_args, resp_kwargs = parameter_vilify(request, 'ids', 'types', values=None)
    ids, types = resp_args
    values = resp_kwargs[0]

    # if types == "LTECell":
    #     resp = get_response_data("GetEuInfoMessage", {
    #         "getEuInfoType": "EU_RU_LTECELL",
    #         "getEuInfoInstance": ids
    #     })
    #     cls_obj = MoreFormatConversion(resp, page, page_size)
    #     content = cls_obj.data_reconstitution()
    #     return jsonify(content)    后期备用

    if types == "RFChannel":  # 借用已经写好的types来作为RFChannel的接口
        resp = get_response_data("GetEuInfoMessage", {
            "getEuInfoType": "EU_RU_RFChannel",
            "getEuInfoInstance": ids
        })
        cls_obj = MoreFormatConversion(resp, page, page_size)
        content = cls_obj.data_reconstitution()
        return jsonify(content)

    if types == "RU":
        ids = re.sub('RU_', '', ids)
        if VersionCheck():
            resp = get_response_data("GetEuInfoMessage", {
                "getEuInfoType": "RuInfo",
                "getEuInfoInstance": ids + "."
            })
        else:
            resp = GenerateRUParamFromCustom(ids)

    elif types == "getRRUPowerSwitch":
        ids = re.sub('RU_', '', ids)
        if VersionCheck():
            resp = get_response_data("GetEuInfoMessage", {
                "getEuInfoType": "RuInfo",
                "getEuInfoInstance": ids + "."
            })
        else:
            resp = GenerateRUParamFromCustom(ids)

        for item in resp.get("parametersArry"):
            parameterName = item.get("parameterName")
            part1 = re.search("RU", parameterName)
            part2 = re.search("RouteIndex", parameterName)
            if part1 is not None and part2 is not None:
                if part1.group(0) == "RU" and part2.group(0) == "RouteIndex":
                    parameterValue = item.get("parameterValue")
                    break

        if parameterValue is None:
            parameterValue = "1.1+1"

        # resp = get_response_data("GetEuInfoMessage", {
        #     "getEuInfoType": types,
        #     "getEuInfoInstance": ids + "."
        # })
        # read RRU Powersume data file in bbu local path

        logFile = request.args.get("logFile")
        logFile = logFile.replace("?RouteIndex?", parameterValue)
        logFileName = os.path.basename(logFile)
        logFileDir = logFile.split("/" + logFileName)[0]
        PowerConsumeDataList = get_PowersumeDataFromFile(logFileDir, logFileName)

        resp = ""
        if len(resp) <= 0:
            resp = dict()
            resp["MSG_TYPE"] = "getRRUPowerSwitchMessageResponse"
            resp["RouteIndex"] = parameterValue
            switchParam = list()
            paramDict = dict()
            paramDict["parametersName"] = ""
            paramDict["parametersValue"] = "0"
            switchParam.append(paramDict)
            resp["PowerConSumeSwitchStatus"] = switchParam
            if PowerConsumeDataList is not None:
                resp["parametersArry"] = PowerConsumeDataList
            else:
                resp["parametersArry"] = []
        elif resp.get("parametersArry") is not None:
            resp["PowerConSumeSwitchStatus"] = list((resp.get("parametersArry"))).copy()
            resp["RouteIndex"] = parameterValue
            (resp.get("parametersArry")).clear()
            if PowerConsumeDataList is not None:
                resp.get("parametersArry").extend(PowerConsumeDataList)
            else:
                resp.get("parametersArry").extend([])
    else:
        if VersionCheck():
            resp = get_response_data("GetEuInfoMessage", {
                "getEuInfoType": "EuInfo",
                "getEuInfoInstance": ids + "."
            })
        else:
            resp = GenerateEUParamFromCustom(ids)

        if values:
            setParameterName = resp.get('parametersArry')[-1].get("parameterName")  # type:str
            params_list = [
                {"parameterName": '.'.join(setParameterName.split('.')[:-1]) + ".ModelName", "parameterValue": values}]
            SetParameterValues(params_list)

    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 参数配置(config)->RU
@acs_data.route('/config_RU', methods=['GET'])
@login_requied
def get_config_ru_data():
    request_params = request.args
    page_size = int(request_params.get("limit", 20))
    page = int(request_params.get("page", 1))
    parameters_list = [
        "Device.DeviceInfo.RU.RxAttenuation",
        "Device.DeviceInfo.RU.TxAttenuation",
        "Device.DeviceInfo.RU.IP",
        "Device.DeviceInfo.RU.AdminState",
        "Device.DeviceInfo.RU.OPState",
        "Device.DeviceInfo.RU.RUOnlineStatus",
        "Device.DeviceInfo.RU.RUVersion",
        "Device.DeviceInfo.RU.BBUMacAddress",
        "Device.DeviceInfo.RU.RRUMacAddress",
        "Device.DeviceInfo.RU.Reboot"
    ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 参数配置(config)->SCMgmt
@acs_data.route('/config_SCMgmt', methods=['GET'])
@login_requied
def get_config_sc_mgmt_data():
    page, page_size, resp_args = parameter_vilify(request, 'index')
    if resp_args[0] == '0':
        types = "SC_COMMON_PARA"
    elif resp_args[0] == '1':
        types = "SC_TIME_CONRL"
    elif resp_args[0] == "2":
        types = "SC_Uu"
    elif resp_args[0] == "3":
        types = "SC_X2"
    elif resp_args[0] == "4":
        types = "SC_XN"
    else:
        types = "SC_Uu_EXTEND"

    resp = get_response_data("GetSCMessage", {
        "MsgType": types,
        "MsgInstance": "Device.FAP.SCMgmt.Config."
    })
    cls_obj = MoreFormatConversion(resp, page, page_size)
    content = cls_obj.data_reconstitution()
    return jsonify(content)


# 参数配置(config)->SCMgmt->性能管理(SC)软采参数管理
@acs_data.route('/config_SCMgmt_SC', methods=["GET"])
@login_requied
def get_sc_data():
    page, page_size = parameter_vilify(request)
    parameters_list = [
        "Device.FAP.SCMgmt.Config.1.SCEnable",
        "Device.FAP.SCMgmt.Config.1.SCIPAddress",
        "Device.FAP.SCMgmt.Config.1.SCPort",
        "Device.FAP.SCMgmt.Config.1.SCLoginID",
        "Device.FAP.SCMgmt.Config.1.SCPassword",
        "Device.FAP.SCMgmt.Config.1.SCNCGIList",
        "Device.FAP.SCMgmt.Config.1.SCGID",
        "Device.FAP.SCMgmt.Config.1.P2PNEMode",
        "Device.FAP.SCMgmt.Config.1.SamplePeriod",
        "Device.FAP.SCMgmt.Config.1.MaxUEs",
    ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 参数配置(config)->SCMgmt->软采参数管理
@acs_data.route('/config_SCMgmt_Uu-extend', methods=["GET"])
@login_requied
def get_Uu_extend_data():
    page, page_size = parameter_vilify(request)
    parameters_list = [
        "Device.FAP.SCMgmt.Config.1.UuExtendEnable",
        "Device.FAP.SCMgmt.Config.1.UuExtendAllEnable",
        "Device.FAP.SCMgmt.Config.1.UuExtendPHREnable",
        "Device.FAP.SCMgmt.Config.1.UuExtendULSINREnable",
        "Device.FAP.SCMgmt.Config.1.UuExtendTAEnable",
        "Device.FAP.SCMgmt.Config.1.UuExtendCQIEnable",
        "Device.FAP.SCMgmt.Config.1.UuExtendgNBReceivedInterfereEnable",
        "Device.FAP.SCMgmt.Config.1.UuExtendDtchPrbAssnUlEnable",
        "Device.FAP.SCMgmt.Config.1.UuExtendDtchPrbTotUlEnable",
        "Device.FAP.SCMgmt.Config.1.UuExtendDtchPrbAssnDlEnable",
        "Device.FAP.SCMgmt.Config.1.UuExtendDtchPrbTotDlEnable",
    ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 参数配置(config)->LocalBreakOut 已经隐藏  http://127.0.0.1:5000/acs/settings/LocalBreakOut
@acs_data.route('/config_LocalBreakOut', methods=['GET'])
@login_requied
def get_config_local_break_out_data():
    page, page_size, resp_args = parameter_vilify(request, 'index')
    if resp_args[0] == '0':
        resp = GetParameterValues(['Device.Services.FAPService.1.FAPControl.LocalBreakout.Enable'])
        cls_obj = FormatConversion(resp, page_size, page)
        content = cls_obj.resp_to_dict()
        return jsonify(content)
    if resp_args[0] == '1':
        types = "DnsRules"
    elif resp_args[0] == '2':
        types = "IpRules"
    elif resp_args[0] == "3":
        types = "NssaiRules"
    elif resp_args[0] == '4':
        types = "PlmnRules"
    elif resp_args[0] == '5':  # 临时新增,可删
        types = "TrafficRules"
    elif resp_args[0] == '6':  # 临时新增,可删
        types = "TrafficStatistics"
    else:
        abort(400)
    resp = get_response_data("GetLocalBreakOutMessage", {
        "MsgType": types,
        "MsgInstance": "Device.Services.FAPService.1."
    })
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 参数配置(config)->LocalBreakOutPop
@acs_data.route('/config_LocalBreakOutPop', methods=['GET'])
@login_requied
def get_config_local_break_out_data_pop():
    page, page_size, resp_args = parameter_vilify(request, 'types', "ids")
    resp = get_response_data("GetLocalBreakOutMessage", {
        "MsgType": resp_args[0],
        "MsgInstance": resp_args[1]
    })
    cls_obj = FormatConversion(resp, page_size, page)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 文件上传校验, 校验上传的/opt/images目录是否大于550MB
@acs_data.route('/upload_disk_size', methods=["GET"])
@login_requied
def upload_disk_size():
    status_data = cookie_get().get("upload")
    request_data = request.args
    file_name = request_data.get("fileName", '')
    ids = request_data.get("ids", '')
    if file_name == '':
        return jsonify(cookie_get().get("msg", {}).get("402"))
    try:
        if ids == 'ac_file':
            disk_size = psutil.disk_usage('/etc').free / 1024 / 1024
            if disk_size < 5:
                return jsonify({"msg": status_data.get("lt"), "code": 400})
        else:
            disk_size = psutil.disk_usage('/tmp').free / 1024 / 1024 / 1024
            if disk_size < 0.55:
                return jsonify({"msg": status_data.get("lt"), "code": 400})
            if os.path.exists('/tmp/%s' % file_name):
                return jsonify({"msg": status_data.get("The file all ready exists"), "code": 400})
    except:
        return jsonify({"msg": status_data.get("absence"), "code": 400})
    return jsonify(code=200, msg="OK")


# 文件上传(fileUpload)
@acs_data.route('/upload', methods=['POST'])
@login_requied
def upload():
    msg = cookie_get().get("msg", {})
    request_data = request.form.to_dict()
    types = request_data.get("types")
    level = request_data.get("level", '')
    img_list = dict(request.files)
    if request.method == "POST" and 'photo' in img_list:
        if types == "uploadLicense":
            if os.path.exists('/root/5g.license'):
                filename = '/root/5g.license.bk'
                if os.path.exists(filename):
                    os.remove(filename)
                    os.rename('/root/5g.license', '/root/5g.license.bk')
                    photos.save(request.files['photo'], folder='/root', name='5g.license')
                else:
                    os.rename('/root/5g.license', '/root/5g.license.bk')
                    photos.save(request.files['photo'], folder='/root', name='5g.license')
            else:
                # filename = request.files['photo'].filename
                photos.save(request.files['photo'], folder='/root', name='5g.license')
            resp = get_response_data("LicenseMessage",
                                     {"LicenseMethod": "ImportLicense", "LicensePath": "/root/5g.license"})
            if resp is None or not isinstance(resp, dict):
                msg = msg.get("400")
            else:
                if "ImportLicenseStatus" in resp.keys():
                    msg = {"msg": "OK", "code": 200, "status": resp.get("ImportLicenseStatus")}
                else:
                    msg = msg.get("400")
        elif types == "uploadACFile":
            if level == 'rsa' or level == 'x509' or level == 'x509ca':
                if not os.path.exists('/etc/swanctl/' + level):
                    os.mkdir('/etc/swanctl/' + level, 755)
            photos.save(request.files['photo'], folder='/etc/swanctl/' + level)
            msg = {"msg": "OK", "code": 200, "status": True}
        else:
            filename = photos.save(request.files['photo'])
            resp = get_response_data("GetUploadMessage",
                                     {"UploadStatus": "SUCESS",
                                      "UploadPath": "/tmp/%s" % filename})
            status = resp.get("UpGradeStatus")
            if status == "Upgrading":
                msg = {"msg": "OK", "code": 200, "status": status}
            else:
                msg = {"msg": "OK", "code": 400, "status": status}
    else:
        msg = msg.get("400")
    return jsonify(msg)


# 上传实时状态获取, 判断是否上传成功
@acs_data.route('/upload_status', methods=["GET"])
@login_requied
def upload_status():
    resp = get_response_data("CheckUpgradeStatus", {})
    if resp is None or not isinstance(resp, dict):
        return jsonify({"code": 400, "msg": "OK", "status": "Upgrade Error"})
    status = resp.get("UpGradeStatus")
    if status == "Upgrading":
        msg = {"code": 200, "msg": "OK", "status": status}
    else:
        msg = {"code": 400, "msg": "OK", "status": status}
    return jsonify(msg)


# 下载校验, 返回文件列表
@acs_data.route('/download_verify', methods=['POST', 'GET'])
@login_requied
def download_verify():
    msg = cookie_get().get("msg", {})
    page, limit, resp_kwargs = parameter_vilify(request, fileType="LogFile")
    file_type = resp_kwargs[0]
    resp = get_response_data("GetDownloadMessage", {"getDownload": file_type})
    cls_obj = FormatConversion(resp, page_size=limit, page=page)
    check_result = cls_obj.data_check()
    if check_result.get("status"):
        resp_array = check_result.get("content")
        file_list = list()
        for file in resp_array:
            file_path = file.get("parameterName")  # type:str
            if file_path == "Failed":
                return jsonify(msg.get("406"))
            elif file_path == "Downloading":
                return jsonify(msg.get("202"))
            else:
                if os.path.exists(file_path):
                    file_to_list(file_path, file_list)
        content = {"code": 200, "msg": "OK", "count": len(file_list),
                   "data": file_list[(page - 1) * limit:page * limit]}
    else:
        content = msg.get("204")
    return jsonify(content)


# 文件下载(download)
@acs_data.route('/download', methods=['GET'])
@login_requied
def download():
    rd = request.args
    source = rd.get("source")  # source:str
    if source != "ANR":
        file_name = rd.get("fileName")  # type:str
        split_list = file_name.split('/')
        path = '/'.join(split_list[0:-1])
        resp = make_response(send_from_directory(path, split_list[-1], as_attachment=True))
    else:
        file_name = rd.get("fileName")  # type:str
        path = "/opt/jwinx/oam/Log/"
        if os.path.exists(path + file_name):
            os.system("cd " + path + " && rm -rf " + file_name)
            os.system("cd " + path + " && zip -q -9 " + file_name + " " + file_name.split(".")[0] + ".txt ")
        else:
            os.system("cd " + path + " && zip -q -9 " + file_name + " " + file_name.split(".")[0] + ".txt ")
        resp = make_response(send_from_directory(path, file_name, as_attachment=True))

    if rd.get("types") not in ["log", "identity"]:
        get_response_data("GetDownloadRspMessage", {"DownloadStatus": "DownLoadSucess", "DownloadFileName": file_name})
    return resp


# 健康(Health, 暂时隐藏了)
@acs_data.route('/health_data')
@login_requied
def get_health_data():
    lan_data = cookie_get().get("son").get("tree_title")
    resp = get_response_data("GetHealthMessage", {})
    if resp is None:
        return {"content": [], "code": 400, "msg": "Error", "count": 0}
    data_arrays = resp.get("parametersArry")
    device_info = list()
    fap_dist = dict()
    du_dist = dict()
    # 取出每一个对象
    for health_dict in data_arrays:
        # 分割
        spilt_list = health_dict.get("parameterName").split(".")
        # device_info
        if spilt_list[-3] == "Device":
            device_info.append(health_dict)
        # FAPService
        elif spilt_list[-3] == "FAPControl":
            fap_key = spilt_list[-1].split("_")
            fap_number = lan_data + "_" + fap_key[-1]
            if fap_number not in fap_dist:
                fap_dist[fap_number] = {}
            temp_fap_dist = {}
            if fap_key[-2] not in fap_dist.get(fap_number):
                temp_fap_dist[fap_key[-2]] = health_dict.get("parameterValue")
                fap_dist[fap_number].update(temp_fap_dist)
        else:
            fap_key = spilt_list[-1].split("_")
            du_number = fap_key[0] + "_" + fap_key[1]
            if du_number not in du_dist:
                du_dist[du_number] = {}
            temp_fap_dist = {}
            if fap_key[-3] == "DU":
                if "du_status" not in du_dist.get(du_number):
                    temp_fap_dist["du_status"] = health_dict.get("parameterValue")
                    du_dist[du_number].update(temp_fap_dist)
            else:
                if fap_key[-4] + fap_key[-2] not in du_dist.get(du_number):
                    temp_fap_dist[fap_key[-4] + "_" + fap_key[-2]] = health_dict.get("parameterValue")
                    du_dist[du_number].update(temp_fap_dist)
    data = {"device": device_info, "FAPService": fap_dist, "duData": du_dist}
    return jsonify({"code": 200, "data": data})


# 图表界面(chart)
@acs_data.route('/cpu_performance', methods=['GET'])
@login_requied
def cpu_performance():
    # cpu
    cpu = psutil.cpu_percent(1)
    # 内存使用率
    mem = psutil.virtual_memory()
    total = float(mem.total)
    used = float(mem.used)
    memory_utilization = used / total
    memory_utilization = "%.2f" % (memory_utilization * 100)
    use = 0
    free = 0
    disk = psutil.disk_partitions()
    for i in disk:
        if i.opts != 'cdrom':
            curr_disk_info = psutil.disk_usage(i.mountpoint)
            use += curr_disk_info.used / 1024 / 1024 / 1024
            free += curr_disk_info.free / 1024 / 1024 / 1024
    return jsonify({
        "code": 200,
        "msg": "OK",
        "cpu": cpu,
        "memoryUtilization": memory_utilization,
        "diskUse": "%.2f" % use,
        "diskFree": "%.2f" % free
    })


# 重启(reboot)
@acs_data.route('/reboot', methods=["GET"])
@login_requied
def device_reboot():
    get_response_data("GetRebootMessage", {})
    return jsonify(code=200, msg="OK")


# 判断是否pyacs是否运行成功(运行成功代表重启成功)
@acs_data.route('/is_connected', methods=["GET"])
@login_requied
def is_connected():
    if system_time():
        session.pop('username', None)
        session.pop('usertype', None)
    return jsonify(code=200, msg="OK")


# 重置(reset)
@acs_data.route('/reset', methods=["GET"])
@login_requied
def device_reset():
    resp = get_response_data("GetFactoryResetMessage", {})
    if resp is None or not isinstance(resp, dict):
        return jsonify({"code": 400, "msg": "OK", "status": "Reset Error"})
    status = resp.get("FactoryResetStatus")
    if status == "FactoryReseting":
        msg = {"code": 200, "msg": "OK", "status": status}
    else:
        msg = {"code": 400, "msg": "OK", "status": status}
    return jsonify(msg)


# user类型用户接口
# 安装向导(install)
@acs_data.route('/user_install', methods=['POST'])
@login_requied
def user_install():
    msg = cookie_get().get("msg", {})
    request_data = request.json
    ipAddress = request_data.get("ipAddress")
    primaryPLMN = request_data.get("primaryPLMN")
    NRARFCNDL = request_data.get("NRARFCNDL")
    PhyCellID = request_data.get("PhyCellID")
    SecGWServer1 = request_data.get("SecGWServer1")
    SecGWServer2 = request_data.get("SecGWServer2")
    SecGWServer3 = request_data.get("SecGWServer3")

    if not all([ipAddress, primaryPLMN, NRARFCNDL, PhyCellID, SecGWServer1, SecGWServer2, SecGWServer3]):
        return jsonify(msg.get("402", {}))
    params_list = [
        {"parameterName": "Device.Services.FAPService.1.FAPControl.NR.AMFPoolConfigParam.1.LocalUpIPAddress",
         "parameterValue": ipAddress},
        {"parameterName": "Device.Services.FAPService.1.FAPControl.NR.AMFPoolConfigParam.1.PLMNID",
         "parameterValue": primaryPLMN},
        {"parameterName": "Device.Services.FAPService.1.CellConfig.1.NR.RAN.RF.NRARFCNDL", "parameterValue": NRARFCNDL},
        {"parameterName": "Device.Services.FAPService.1.CellConfig.1.NR.RAN.RF.PhyCellID", "parameterValue": PhyCellID},
        {"parameterName": "Device.IPsec.Gateway.SecGWServer1",
         "parameterValue": SecGWServer1},
        {"parameterName": "Device.IPsec.Gateway.SecGWServer2",
         "parameterValue": SecGWServer2},
        {"parameterName": "Device.IPsec.Gateway.SecGWServer3",
         "parameterValue": SecGWServer3},
    ]
    resp = SetParameterValues(params_list)
    if resp is None or resp.get("status") is None:
        return jsonify(msg.get("501", {}))
    if resp.get("status") == "0":
        return jsonify(msg.get("406", {}))
    return jsonify(msg.get("200", {}))


# 设备信息(deviceInfo)
@acs_data.route('/user_deviceInfo', methods=['GET'])
@login_requied
def user_device_info():
    parameters_list = [
        'Device.DeviceInfo.MU.1.HardwareVersion',
        'Device.DeviceInfo.MU.1.SoftwareVersion',
        'Device.Services.FAPService.1.CellConfig.1.NR.RAN.OpState'
    ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


# 设备管理(deviceConfig)
@acs_data.route('/user_deviceConfig', methods=['GET'])
@login_requied
def user_device_config():
    parameters_list = [
        'Device.Ethernet.Interface.1.Status',
    ]
    resp = GetParameterValues(parameters_list)
    cls_obj = FormatConversion(resp)
    content = cls_obj.resp_to_dict()
    return jsonify(content)


@acs_data.route('/download_identity', methods=["GET"])
@login_requied
def get_download_identity():
    lan_data = cookie_get().get("download_identity")
    resp = get_response_data("GetDownloadMessage", {"getDownload": "LicenseIdentity"})
    if resp is not None and isinstance(resp, dict):
        if "IdentityPath" in resp.keys():
            return jsonify({"code": 200, "path": resp.get("IdentityPath"), "msg": "OK"})
        else:
            return jsonify({"code": 400, "data": None, "msg": lan_data.get("failed")})
    else:
        return jsonify({"code": 400, "data": None, "msg": lan_data.get("error")})


@acs_data.route('/caught', methods=['POST'])
@login_requied
def caught():
    msg = cookie_get().get("msg", {})
    data = request.json

    types = data.get("types")
    options = data.get("SelectMsg")

    if not all([types, options]):
        return jsonify(msg.get("400", {}))

    ip = data.get("TransmitIp", '')
    if types == "start":
        request_dict = {
            "Purpose": "StartTcpdump",
            "TransmitIp": ip,
            "SelectMsg": options
        }
    else:
        request_dict = {
            "Purpose": ip,
            "TransmitIp": '',
            "SelectMsg": options
        }
    resp = get_response_data("WebTcpDump", request_dict)
    if not isinstance(resp, dict):
        return jsonify(msg.get("406", {}))
    status = resp.get("TcpdumpStatus", '')
    if status == '' or "SUCCESS" not in status:
        content = msg.get("406", {})
    else:
        content = {"code": 200, "data": resp, "msg": "OK"}
    return jsonify(content)


# 系统设置
@acs_data.route('/system_config', methods=['POST'])
@login_requied
@user_authenticate("super")
def update_system_config():
    msg = cookie_get().get("msg", {})
    lang = request.cookies.get("lan_type")
    username = session.get("username")
    data = request.json
    login_title_cn = data.get("loginTitleCN", '')
    login_title_en = data.get("loginTitleEN", '')
    header_title_cn = data.get("headerTitleCN", '')
    header_title_en = data.get("headerTitleEN", '')
    company_cn = data.get("companyCN", '')
    company_en = data.get("companyEN", '')
    version_cn = data.get("versionCN", '')
    version_en = data.get("versionEN", '')
    url = data.get("CURLInfo", '')
    copyrights = data.get("CopyRight", '')
    login_time_out = int(data.get("loginTimeOut", 1))
    inner_id = session.get("id")
    client_id = request.cookies.get('id')
    cookie_get()
    if not all([login_time_out, inner_id, client_id]):
        return jsonify(msg.get("402", {}))
    if inner_id == des_descrypt(client_id, des_publicKey()):
        if not set_session_time_out(login_time_out):
            return jsonify(msg.get("417", {}))
        if not set_cookie_time_out(login_time_out):
            return jsonify(msg.get("417", {}))
    else:
        return jsonify(msg.get("502", {}))
    if [login_title_cn, login_title_en, header_title_cn, header_title_en, company_cn, company_en, version_cn,
        version_en].count('') >= 8:
        return jsonify({"code": 211, "msg": "OK"})

    file_obj = FileOperations
    if lang == 'cn' and login_title_cn != '' or header_title_cn != '' or company_cn != '' or version_cn != '' or url != '' or copyrights != '':
        config_path = root_path + "/static/language/lan_ch.json"
        config_data = file_obj.read_json_file(config_path)  # type: dict
        if login_title_cn != '':
            config_data["index"]["login_title"] = login_title_cn
        if header_title_cn != '':
            config_data["base"]["cpe_base_station"] = header_title_cn
        if company_cn != '':
            config_data["base"]["companyName"] = company_cn
        if version_cn != '':
            config_data["base"]["version"] = version_cn
        if url != '':
            config_data["base"]["url"] = url
        if copyrights != '':
            config_data["base"]["copyrights"] = copyrights
        file_obj.update_config_file(config_path, config_data)
    if lang == 'en' and login_title_en != '' or header_title_en != '' or company_en != '' or version_en != '' or url != '' or copyrights != '':
        config_path = root_path + "/static/language/lan_en.json"
        config_data = file_obj.read_json_file(config_path)  # type: dict
        if login_title_en != '':
            config_data["index"]["login_title"] = login_title_en
        if header_title_en != '':
            config_data["base"]["cpe_base_station"] = header_title_en
        if company_en != '':
            config_data["base"]["companyName"] = company_en
        if version_en != '':
            config_data["base"]["version"] = version_en
        if url != '':
            config_data["base"]["url"] = url
        if copyrights != '':
            config_data["base"]["copyrights"] = copyrights
        file_obj.update_config_file(config_path, config_data)

    return jsonify({"code": 200, "msg": "OK"})


@acs_data.route('/system_config/upload_logo', methods=['POST'])
@login_requied
def upload_logo():
    logo_data = request.files['photo']
    file_obj = FileOperations()
    style_dirs = file_obj.read_template_list()
    i = 0
    first_file = ''
    for item in style_dirs:
        img_logo = root_path + "/static/style/" + item + "/images/logo.png"
        if os.path.exists(img_logo):
            os.remove(img_logo)
            time.sleep(0.5)
        if i == 0:
            first_file = img_logo
            photos.save(logo_data, folder=root_path + "/static/style/" + style_dirs[i] + "/images/", name='logo.png')
        else:
            target = root_path + "/static/style/" + item + "/images"
            shutil.copy(first_file, target)
        i = i + 1

    return jsonify({"code": 200, "msg": "OK"})


@acs_data.route('/system_config_get_data', methods=['GET'])
@login_requied
def get_system_config():
    content = dict()
    file_obj = FileOperations
    cn_data = file_obj.read_json_file(root_path + "/static/language/lan_ch.json")
    content["loginTitleCN"] = cn_data["index"]["login_title"]
    content["headerTitleCN"] = cn_data["base"]["cpe_base_station"]
    content["companyCN"] = cn_data["base"]["companyName"]
    content["versionCN"] = cn_data["base"]["version"]
    content["CopyRight"] = cn_data["base"]["copyrights"]
    content["CURLInfo"] = cn_data["base"]["url"]
    # en_data = file_obj.read_json_file(root_path + "/static/language/lan_en.json")
    # content["loginTitleEN"] = en_data["index"]["login_title"]
    # content["headerTitleEN"] = en_data["base"]["cpe_base_station"]
    # content["companyEN"] = en_data["base"]["companyName"]
    # content["versionEN"] = en_data["base"]["version"]
    session_time_out = get_session_time_out()
    content["login_time_out"] = session_time_out
    return jsonify({"code": 200, "msg": "OK", "data": content})


# 关闭Web服务
@acs_data.route('/system_config/web_server_shutdown', methods=['POST'])
@login_requied
@user_authenticate("super")
def set_webserver_shutdown():
    request_params = json.loads(str(request.data, "utf-8"))
    params_list = {
        "parametersArry": [{"parameterName": "web_server_on", "parameterValue": request_params["web_server_on"]}]}

    resp = get_response_data("SetWebSvrShutdownMessage", params_list)
    if resp is not None and isinstance(resp, dict):
        cls_obj = FormatConversion(resp)
        content = cls_obj.resp_to_dict()

        if int(content.get("status")) == 1:
            return jsonify({"code": 200, "msg": "OK,成功关闭Web管理功能！", "data": content})
    return jsonify({"code": 500, "msg": "Error,关闭Web管理失败！", "data": {}})


# 用户类型
@acs_data.route('/user_types', methods=['GET'])
@login_requied
def get_user_types():
    user_types = FileOperations()
    user_types_list = user_types.read_user_types_info()
    return jsonify({"code": 200, "msg": "OK", "data": user_types_list})


# 用户名称列表
@acs_data.route('/user_lists', methods=['GET'])
@login_requied
def get_user_list():
    fileOperation = FileOperations()
    user_list = fileOperation.read_user_list()
    return jsonify({"code": 200, "msg": "OK", "data": user_list})


# 模板样式
@acs_data.route('/template_type', methods=['GET'])
@login_requied
def get_template_types():
    template_type = FileOperations()
    template_type_list = template_type.read_template_list()
    return jsonify({"code": 200, "msg": "OK", "data": template_type_list})


# 接收websocket连接
@acs_data.route('/ws', methods=["GET", "SET"])
@login_requied
def ws():
    # 建立连接 type:WebSocket
    ws_conn = request.environ.get('wsgi.websocket')
    if not ws_conn:
        return None

    current_user_id = session['user_info']['id']
    add_websocket_clients(current_user_id, ws_conn)


# 标准权限菜单列表
@acs_data.route('/menu_role_tree', methods=["GET"])
@login_requied
@user_authenticate("super")
def get_menu_role_tree():
    menus_handle = FileOperations()
    return jsonify({"code": 200, "msg": "OK", "data": menus_handle.read_standard_menu_list()})


# 用户权限菜单列表
@acs_data.route('/user_menu_role_tree', methods=["GET"])
@login_requied
def get_user_role_tree():
    menus_handle = FileOperations()
    # 拿取请求数据
    request_params = request.args
    # 获取需要添加的用户类型
    userid = request_params.get("id", '')
    return jsonify({"code": 200, "msg": "OK", "data": menus_handle.read_user_role_menu_list(userid)})


# 当前用户权限菜单列表
@acs_data.route('/current_user_roles', methods=["GET"])
@login_requied
def get_current_user_roles():
    # 拿取中英文词典
    msg = cookie_get().get("msg", {})
    if not system_time():
        return jsonify(msg.get("503", {}))
    # 拿取请求数据
    request_params = request.args
    # 获取需要添加的用户类型
    user_id = request_params.get("id")
    username = request_params.get("username")
    if not all([user_id, username]):
        return jsonify(msg.get("402", {}))
    if not all([session.get("username")]) or session.get("username") != username:
        return jsonify(msg.get("503", {}))

    data_handle = FileOperations()
    uid = des_descrypt(user_id, des_publicKey())
    return jsonify({"code": 200, "msg": "OK", "data": data_handle.read_current_user_roles(uid)})


@acs_data.route('/user/Options', methods=["GET"])
@login_requied
def get_user_options():
    # 拿取中英文词典
    msg = cookie_get().get("msg", {})
    if not system_time():
        return jsonify(msg.get("503", {}))
    # 拿取请求数据
    request_params = request.args
    # 获取需要添加的用户类型
    user_id = request_params.get("id")
    username = request_params.get("username")
    if not all([user_id, username]):
        return jsonify(msg.get("402", {}))
    if not all([session.get("username")]) or session.get("username") != username:
        return jsonify(msg.get("503", {}))
    uuid = des_descrypt(user_id, des_publicKey())
    if not all([session.get("id"), user_id]) or uuid != session.get("id"):
        return jsonify(msg.get("503", {}))
    data_handle = FileOperations()
    return jsonify({"code": 200, "msg": "OK", "data": data_handle.read_user_options(uuid)})


@acs_data.route('/debug/device/actions', methods=["POST"])
@login_requied
def debug_device():
    data = request.json
    result_array = list()
    deny = base64_decode(session.get("deny"))[slice(None, None, -1)]
    permit = base64_decode(session.get("permit"))[slice(None, None, -1)]
    access = base64_decode(session.get("access"))[slice(None, None, -1)]

    command = des_decrypt_form_js(data.get("command"), deny, permit, access)
    decrypt_data = invalid_character_escaped(command)

    response_data = get_response_data("getDebugMessage", {"WEBCMD": decrypt_data})
    sf = StringIO(response_data.get("rspMsg"))
    while True:
        value = sf.readline()
        if value != '':
            result_array.append(des_encrypt_form_js(value, deny, permit, access))
        else:
            break
    sf.close()
    resp = jsonify({"code": 200, "msg": "OK", "data": result_array})
    push_encrypt_code(resp)
    return resp


@acs_data.route('/param/part/nxcTebDqLeuihLegdWg3ct9ntfd1zA3MP5Ep60SRNr6zkRt1s4kAQ8h1v7NcGYNd', methods=["POST"])
@login_requied
def get_param_data():
    request_params = request.json
    cookie_key = request_params.get("key")
    if cookie_key is not None and isinstance(cookie_key, list):
        data = []
        for item in cookie_key:
            data.append(request.cookies.get(item))
    elif cookie_key is not None:
        data = request.cookies.get(cookie_key)
    else:
        data = ""
    return jsonify({"code": 200, "msg": "OK", "data": data})


@csrf_handle.exempt
@acs_data.route('/smallcell/config', methods=["POST"])
@api_requied
def set_small_cell_info():
    msg = cookie_get().get("msg", {})

    # 接收参数
    # request_params = request.json
    json_str = str('')
    request_params = request.form
    json_dict = request_params.to_dict(flat=False)
    for key, value in json_dict.items():
        json_str = key
        break
    json_data = json.loads(json_str)

    _gNBId = json_data.get("cellGNBId")
    _centerFrequency = json_data.get("cellCenterFrequency")
    _dlBandWidth = json_data.get("cellDLBandWidth")
    _slotRatio = json_data.get("cellSlotRatio")
    _signalPower = json_data.get("cellSignalPower")

    print('\n--------small cell config--------\n')
    print('cellGNBId=', str(_gNBId))
    print('cellCenterFrequency=', str(_centerFrequency))
    print('cellDLBandWidth=', str(_dlBandWidth))
    print('cellSlotRatio=', str(_slotRatio))
    print('cellSignalPower=', str(_signalPower))
    print('-----------------------------------\n')

    # 检测参数值范围
    params_list = [
        {"parameterName": "Device.Services.FAPService.1.FAPControl.NR.RAN.Common.gNBId", "parameterValue": str(_gNBId)},
        {"parameterName": "Device.Services.FAPService.1.CellConfig.1.PrivateExtension.centerFrequency",
         "parameterValue": str(_centerFrequency)},
        {"parameterName": "Device.Services.FAPService.1.CellConfig.1.NR.RAN.RF.DLBandwidth",
         "parameterValue": str(_dlBandWidth)},
        {"parameterName": "Device.Services.FAPService.1.CellConfig.1.PrivateExtension.slotRatio",
         "parameterValue": str(_slotRatio)},
        {"parameterName": "Device.Services.FAPService.1.CellConfig.1.PrivateExtension.signalPower",
         "parameterValue": str(_signalPower)}
    ]
    resp = SetParameterValues(params_list)

    # 判断返回是否成功
    if resp is None:
        resp_msg = jsonify({'code': 404, 'msg': '配置小区参数失败', 'status': '-1'})
    else:
        _status = resp.get("status")
        if _status == "1":
            print("{'code': '200', 'msg': '配置小区参数成功', 'status': '1'}\n")
            resp_msg = jsonify({'code': 200, 'msg': '配置小区参数成功', 'status': _status})
        else:
            print("{'code': '500', 'msg': '配置小区参数失败', 'status': '0'}\n")
            resp_msg = jsonify({"code": 500, "msg": '配置小区参数失败', "status": _status})
    return resp_msg


@acs_data.route('/smallcell/query', methods=["GET"])
@api_requied
def get_small_cell_info():
    cell_info = dict()
    _gNBId = 0
    _centerFrequency = 0
    _dlBandWidth = 0
    _slotRatio = 0
    _signalPower = 0
    parameters_list = [
        "Device.Services.FAPService.1.FAPControl.NR.RAN.Common.gNBId",
        "Device.Services.FAPService.1.CellConfig.1.PrivateExtension.centerFrequency",
        "Device.Services.FAPService.1.CellConfig.1.NR.RAN.RF.DLBandwidth",
        "Device.Services.FAPService.1.CellConfig.1.PrivateExtension.slotRatio",
        "Device.Services.FAPService.1.CellConfig.1.PrivateExtension.signalPower"
    ]

    # 发送GetParameterValues请求拿取参数值
    msg = {'code': '500', 'msg': '查询小区参数失败', 'data': cell_info}
    resp = GetParameterValues(parameters_list)
    if resp is not None:
        parametersArry = resp.get("parametersArry")
        for item in parametersArry:
            if item.get("parameterName") == parameters_list[0]:
                _gNBId = item.get("parameterValue")
            elif item.get("parameterName") == parameters_list[1]:
                _centerFrequency = item.get("parameterValue")
            elif item.get("parameterName") == parameters_list[2]:
                _dlBandWidth = item.get("parameterValue")
            elif item.get("parameterName") == parameters_list[3]:
                _slotRatio = item.get("parameterValue")
            elif item.get("parameterName") == parameters_list[4]:
                _signalPower = item.get("parameterValue")

    cell_info["cellGNBId"] = _gNBId
    cell_info["cellCenterFrequency"] = _centerFrequency
    cell_info["cellDLBandWidth"] = _dlBandWidth
    cell_info["cellSlotRatio"] = _slotRatio
    cell_info["cellSignalPower"] = _signalPower
    if resp is not None:
        msg = {'code': '200', 'msg': '查询小区参数成功', 'data': cell_info}
    resp_msg = jsonify(msg)
    return resp_msg
