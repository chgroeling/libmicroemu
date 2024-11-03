from setuptools import setup, find_packages

setup(
    name="decoder_generator",
    version="0.0.1",
    author="Christian Gröling",
    description="Generator for armv7-m opcode decoder",
    package_data={
        "decoder_generator": ["*.json"],
        "decoder_generator.templates": ["*.j2"],
    },
    include_package_data=True,
    packages=find_packages(),
    install_requires=["jinja2"],
)
