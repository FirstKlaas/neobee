# set base image (host OS)
FROM python:3.8

# set the working directory in the container
RUN mkdir -p /code
WORKDIR /code

# copy the dependencies file to the working directory
COPY requirements.txt .

# install dependencies
RUN python -m pip install --upgrade pip
RUN python -m pip install --upgrade wheel
RUN pip install -r requirements.txt

RUN mkdir -p neobee_relay
# copy the content of the local src directory to the working directory
COPY neobee_relay neobee_relay/
COPY run.py .
COPY idle.py .


ENV PYTHONPATH "${PYTHONPATH}:/code"
# command to run on container start
CMD [ "python", "run.py" ]
