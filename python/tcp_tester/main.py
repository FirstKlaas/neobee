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
    """    
    
    #shell.mqtt_password = None
    #shell.mqtt_login = None
    #shell.mqtt_host = "192.168.178.77"
    #shell.mqtt_port = 1883
    
    #shell.mqtt_password = "encowa"
    #shell.mqtt_login = "encoway"
    #shell.mqtt_host = "vm-web.lenze.digital"
    #shell.mqtt_port = 1883
    
    
    #with open("board.neobee", "w") as f:
    #    json.dump(shell.to_dict(), f, indent=2)
    
    #shell.save_settings()
    print(json.dumps(shell.to_dict(), indent=2))