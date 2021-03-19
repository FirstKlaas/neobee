import logging
import os
from typing import Optional

from dotenv import load_dotenv
from fastapi import FastAPI, HTTPException

from neobee_relay import orm

logger = logging.getLogger(__name__)

app = FastAPI()

@app.on_event("startup")
async def on_startup():
    load_dotenv(verbose=True)
    await orm.init_models(generate_schemas=False)


@app.get("/")
async def read_root():
    return [{
        "mac":b.mac,
        "ip" : b.ip,
        "firmware" : b.firmware
    } for b in await orm.get_boards()]


@app.get("/measurement/{mac}")
async def get_latest_measurement(mac: str):
    if len(mac) != 12:
        logger.error("Invalid mac address. %s", mac)
        raise HTTPException(status_code=404, detail="Board not found")

    macstr = ":".join([mac[i:i+2].upper() for i in range(0, len(mac), 2)])
    measurement = await orm.get_measurement(macstr)

    if measurement is None:
        raise HTTPException(status_code=404, detail=f"No such board {macstr}")

    return measurement


if __name__ == "__main__":
    logger.info("Starting neobee rest api")
