"""
Simple dumper of the neobee board messages
to a human readable form.

Copyright FirstKlaas 2021
"""
import asyncio as aio
from functools import reduce
import logging
import os
from typing import List, Optional

import aio_mqtt
from dotenv import load_dotenv
from tortoise import Tortoise
from tortoise.contrib.pydantic import pydantic_model_creator

from .models.board import BoardModel, Board
from .models.measure import MeasurementModel, Measurement

logger = logging.getLogger(__name__)


async def init_models(create_tables: bool = True):
    # Here we connect to a SQLite DB file.
    # also specify the app name of "models"
    # which contain models from "app.models"
    logger.debug("Connecting to database")
    await Tortoise.init(
        db_url="postgres://neobee:neobee@192.168.178.49:5432/neobee",
        modules={"models": ["neobee_relay.models.board", "neobee_relay.models.measure"]},
    )
    if create_tables:
        await generate_schemas()


async def generate_schemas():
    # Generate the schema
    logger.debug("Generating schema")
    await Tortoise.generate_schemas()


async def get_boards() -> List[Board]:
    try:
        return await Board.from_queryset(BoardModel.all())
    except:
        logger.exception("No Boards")
        return []


async def add_board(board: Board) -> Board:
    await BoardModel.create(**board.dict(exclude_unset=True))


async def get_measurement(mac: str) -> Optional[Measurement]:
    m = await MeasurementModel.get_or_none(mac=mac)
    return None if m is None else await Measurement.from_tortoise_orm(m)
