"""
Simple dumper of the neobee board messages
to a human readable form.

Copyright FirstKlaas 2021
"""
import asyncio as aio
from functools import reduce
import logging
import os

import aio_mqtt
from dotenv import load_dotenv
from tortoise import Tortoise

from .models.board import BoardModel
from .models.measure import MeasurementModel

logger = logging.getLogger(__name__)

async def init_models(generate_schemas: bool = True):
    # Here we connect to a SQLite DB file.
    # also specify the app name of "models"
    # which contain models from "app.models"
    logger.debug("Connecting to database")
    await Tortoise.init(
        db_url="postgres://neobee:neobee@192.168.178.49:5432/neobee",
        modules={"models": ["neobee_relay.models.board", "neobee_relay.models.measure"]},
    )
    if generate_schemas:
        await generate_schemas()

async def generate_schemas():
    # Generate the schema
    logger.debug("Generating schema")
    await Tortoise.generate_schemas()

async def get_boards():
    return await BoardModel.all()

async def get_measurement(mac:str):
    m =  await MeasurementModel.get_or_none(mac=mac)
    if m is None: return None
    return {
        "weight" : m.weight,
        "temperature_inside" : m.temperature_inside,
        "temperature_outside" : m.temperature_outside,
        "timestamp" : m.timestamp
    }