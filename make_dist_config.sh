#!/bin/bash
export DISTRONAME="$(cat distname)"
export DISTROVERSION="$(cat version)"
export DISTRODATE="$(date +%Y-%m-%d)"
export AMIGADATE="$(date +"%-d.%-m.%Y")"
