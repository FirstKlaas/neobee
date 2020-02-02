import json

from neobee.shell import NeoBeeShell

with NeoBeeShell(host="192.168.178.72") as shell:
    #with NeoBeeShell() as shell:

    """
    shell.name = "NeoBee.One"
    shell.ssid = "RepeaterOben24"
    shell.set_password("4249789363748310")
    shell.deep_sleep_seconds = 30
    shell.set_scale_offset(61191.0)
    shell.set_scale_factor(21.88)
    shell.activate_wifi_sta()
    shell.save_settings()
    """
    print(f"mqtt server: {shell.mqtt_host}:{shell.mqtt_port}")
    
    shell.mqtt_password = None
    shell.mqtt_login = None
    shell.mqtt_host = "192.168.178.77"
    shell.mqtt_port = 1882
    
    print(shell.mqtt_password)
    print(shell.wifi_password)
    print(shell.ssid)
    
    # d = shell.to_dict()
    # print(shell.tare(1))
    # print(shell.calibrate(2962,1))
    # print(json.dumps(d, ensure_ascii=True, indent=2))
    # shell.save_settings()

    # shell.reset()
    print(json.dumps(shell.to_dict(), indent=2))