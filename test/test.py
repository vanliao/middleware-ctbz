import requests
import json

addr ="http://" + "192.168.216.135"
# addr ="http://" + "192.168.1.100"

def GetCrypt():
    url = addr + "/crypto/data?" + "type=cbc&param=admin"
    response = requests.post(url)  # json.dumps()将字典解析为字符串
    print(response.text)

def GetVerifyCode():
    url = addr + "/captcha?lang=zh&time=1234"
    response = requests.post(url)  # json.dumps()将字典解析为字符串
    print(response.text)
    body = json.loads(response.text)
    return body["data"]["verifyCode"]

def login(verifyCode):
    url = addr + "/login"
    data = {
        "username": "admin",
        "password": "utaTGv6dkC+OsPPvE3pHPA==",
        "code": verifyCode
    }
    payload = json.dumps(data)
    response = requests.post(url, json=payload)
    print(response.text)
    body = json.loads(response.text)
    return body["data"]["token"]

def logout(token):
    url = addr + "/logout"
    headers = {
        'CUSTOM-PARAM': "van parameters",
        'connection': 'keep-alive',
        'AUTHORIZATION': "Bearer" + token
    }
    response = requests.post(url, headers=headers)
    print(response.text)

def GetShortCutMenu(token):
    url = addr + "/shortcut_menu/data/by_user_id" + "?id=1"
    headers = {
        'CUSTOM-PARAM': "van parameters",
        'connection': 'keep-alive',
        'AUTHORIZATION': "Bearer" + token
    }
    response = requests.post(url, headers=headers)
    print(response.text)

def GetMenu(token):
    url = addr + "/system/menus/data/by_user_id" + "?id=1"
    headers = {
        'CUSTOM-PARAM': "van parameters",
        'connection': 'keep-alive',
        'AUTHORIZATION': "Bearer" + token
    }
    response = requests.post(url, headers=headers)
    print(response.text)

def GetUserInfo(token, user):
    url = addr + "/user/login/info" + "?username=" + user
    headers = {
        'CUSTOM-PARAM': "van parameters",
        'connection': 'keep-alive',
        'AUTHORIZATION': "Bearer" + token
    }
    response = requests.post(url, headers=headers)
    print(response.text)

def GetNrNetwork(token, page, pageSize):
    url = url = addr + "/five_g_system_config/device_manage/data/hm_data" + "?page={}&page_size={}".format(page, pageSize)
    headers = {
        'CUSTOM-PARAM': "van parameters",
        'connection': 'keep-alive',
        'AUTHORIZATION': "Bearer" + token
    }
    response = requests.post(url, headers=headers)
    print(response.text)

def SetNrNetwork(token, name, value):
    url = url = addr + "/five_g_system_config/device_manage/data/hm_data" + "?type=modify"
    headers = {
        'CUSTOM-PARAM': "van parameters",
        'connection': 'keep-alive',
        'AUTHORIZATION': "Bearer" + token
    }
    data = {
        "parameterName": name,
        "parameterValue": value
    }
    payload = json.dumps(data)
    response = requests.post(url, json=payload, headers=headers)
    print(response.text)

def GetNrWan(token, page, pageSize):
    url = url = addr + "/five_g_system_config/device_manage/data/wm_data" + "?type=wan&page={}&page_size={}".format(page, pageSize)
    headers = {
        'CUSTOM-PARAM': "van parameters",
        'connection': 'keep-alive',
        'AUTHORIZATION': "Bearer" + token
    }
    response = requests.post(url, headers=headers)
    print(response.text)

def GetNrVlan(token, page, pageSize):
    url = url = addr + "/five_g_system_config/device_manage/data/wm_data" + "?type=vlan&page={}&page_size={}".format(page, pageSize)
    headers = {
        'CUSTOM-PARAM': "van parameters",
        'connection': 'keep-alive',
        'AUTHORIZATION': "Bearer" + token
    }
    response = requests.post(url, headers=headers)
    print(response.text)

def GetNrDpaa(token, page, pageSize):
    url = url = addr + "/five_g_system_config/device_manage/data/wm_data" + "?type=dpaa&page={}&page_size={}".format(page, pageSize)
    headers = {
        'CUSTOM-PARAM': "van parameters",
        'connection': 'keep-alive',
        'AUTHORIZATION': "Bearer" + token
    }
    response = requests.post(url, headers=headers)
    print(response.text)

def GetNrIpRoute(token, page, pageSize):
    url = url = addr + "/five_g_system_config/device_manage/data/IpRoute" + "?page={}&page_size={}".format(page, pageSize)
    headers = {
        'CUSTOM-PARAM': "van parameters",
        'connection': 'keep-alive',
        'AUTHORIZATION': "Bearer" + token
    }
    response = requests.post(url, headers=headers)
    print(response.text)

def GetNrIpSec(token, page, pageSize):
    url = url = addr + "/five_g_system_config/device_manage/data/im_data" + "?page={}&page_size={}".format(page, pageSize)
    headers = {
        'CUSTOM-PARAM': "van parameters",
        'connection': 'keep-alive',
        'AUTHORIZATION': "Bearer" + token
    }
    response = requests.post(url, headers=headers)
    print(response.text)

addr ="http://" + "192.168.216.135"
# addr ="http://" + "192.168.1.100"

print("--get crypt")
verifyCode = GetCrypt()

print("--get verify code")
verifyCode = GetVerifyCode()

print("--get token")
token = login(verifyCode)

print("--get short cut menu")
GetShortCutMenu(token)

print("--get menu")
GetMenu(token)

print("--get user info")
GetUserInfo(token, "admin")

print("--set nr network")
SetNrNetwork(token, "Device.ManagementServer.ConnectionRequestPassword", "abc")

print("--get nr network")
GetNrNetwork(token, 1, 50)

print("--get nr wan")
GetNrWan(token, 1, 30)

print("--get nr dpaa")
GetNrDpaa(token, 1, 50)

print("--get nr ip route")
GetNrIpRoute(token, 1, 5)

print("--get nr ipsec")
GetNrIpSec(token, 1, 2)

# print("--get nr vlan")
# GetNrVlan(token, 2, 3)
#
# print("--logout")
# logout(token)