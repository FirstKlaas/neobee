import logging
import os
from typing import Optional, List

from dotenv import load_dotenv
from fastapi import FastAPI, HTTPException
from tortoise.contrib.fastapi import HTTPNotFoundError, register_tortoise

from neobee_relay import orm
from neobee_relay.orm import Board, Measurement

logger = logging.getLogger(__name__)

app = FastAPI(title="NeoBee REST API")


@app.on_event("startup")
async def on_startup():
    load_dotenv(verbose=True)
    # await orm.init_models(generate_schemas=False)


@app.get("/boards", response_model=List[Board])
async def read_root() -> List[Board]:
    try:
        return await orm.get_boards()
    except:
        logger.exception("Could not get boards")
        return HTTPException(status_code=500, detail="Could not get boards. Check log.")


@app.get("/measurement/{mac}", response_model=Measurement)
async def get_latest_measurement(mac: str) -> Measurement:
    try:
        if len(mac) != 12:
            logger.error("Invalid mac address. %s", mac)
            raise HTTPException(status_code=404, detail="Board not found")

        macstr = ":".join([mac[i : i + 2].upper() for i in range(0, len(mac), 2)])
        measurement = await orm.get_measurement(macstr)
        if measurement is None:
            raise HTTPException(status_code=404, detail=f"No such board {macstr}")

        return measurement
    except:
        logger.exception("No Measurement")
        return HTTPException(status_code=500, detail="Error requesting measurement.")


register_tortoise(
    app,
    db_url="postgres://neobee:neobee@192.168.178.49:5432/neobee",
    modules={"models": ["neobee_relay.models.board", "neobee_relay.models.measure"]},
    generate_schemas=False,
    add_exception_handlers=False,
)
