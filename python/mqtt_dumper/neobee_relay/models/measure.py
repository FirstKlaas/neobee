from tortoise.models import Model
from tortoise.fields import CharField, FloatField, DatetimeField
from tortoise.contrib.pydantic import pydantic_model_creator


class MeasurementModel(Model):
    mac = CharField(pk=True, max_length=17, description="MAC address as primary key")
    weight = FloatField(null=True)
    temperature_inside = FloatField(null=True)
    temperature_outside = FloatField(null=True)
    timestamp = DatetimeField(auto_now=True)


Measurement = pydantic_model_creator(MeasurementModel, name="Measurement")
