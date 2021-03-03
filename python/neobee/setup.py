import setuptools

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="neobee", # Replace with your own username
    version="0.0.5",
    author="Klaas Nebuhr <FirstKlaas>",
    author_email="klaas.nebuhr@gmail.com",
    description="Condition Monitoring for Bee Hives",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/FirstKlaas/neobee",
    packages=setuptools.find_packages(),
    scripts=['bin/neobee'],
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    python_requires='>=3.6',
)