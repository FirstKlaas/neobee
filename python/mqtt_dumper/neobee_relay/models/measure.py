from tortoise.models import Model
from tortoise.fields import CharField, FloatField, DatetimeField


class MeasurementModel(Model):

    mac = CharField(pk=True, max_length=17, description="MAC address as primary key")
    weight = FloatField(null=True)
    temperature_inside = FloatField(null=True)
    temperature_outside = FloatField(null=True)
    timestamp = DatetimeField(auto_now=True)
