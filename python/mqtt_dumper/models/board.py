from tortoise.models import Model
from tortoise.fields import CharField

class BoardModel(Model):

    mac = CharField(pk=True, max_length=17, description="MAC address as primary key")
    ip = CharField(max_length=15, description="String representation of the local ip")
    firmware = CharField(max_length=12, description="Firmware version")
    name = CharField(max_length=20, description="The non unique name of the board")
