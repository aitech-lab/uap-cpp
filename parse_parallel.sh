#!/bin/bash
ls /storage/fingerprint/data/raw/16/*.tsv.gz \
| parallel \
'zcat {} ' \
'| ./parse ' \
'1> >(gzip >{.}.parsed.gz) ' \
'2> >(gzip >{.}.err.gz)'
