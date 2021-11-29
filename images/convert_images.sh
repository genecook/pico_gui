#!/bin/sh -f

export CONVERT='/usr/bin/convert -verbose -resize 30x30! -rotate 180'

${CONVERT} Chess_kdd40.png kdd30.bmp
${CONVERT} Chess_kdl40.png kdl30.bmp
${CONVERT} Chess_kld40.png kld30.bmp
${CONVERT} Chess_kll40.png kll30.bmp

${CONVERT} Chess_qdd40.png qdd30.bmp
${CONVERT} Chess_qdl40.png qdl30.bmp
${CONVERT} Chess_qld40.png qld30.bmp
${CONVERT} Chess_qll40.png qll30.bmp

${CONVERT} Chess_bdd40.png bdd30.bmp
${CONVERT} Chess_bdl40.png bdl30.bmp
${CONVERT} Chess_bld40.png bld30.bmp
${CONVERT} Chess_bll40.png bll30.bmp

${CONVERT} Chess_ndd40.png ndd30.bmp
${CONVERT} Chess_ndl40.png ndl30.bmp
${CONVERT} Chess_nld40.png nld30.bmp
${CONVERT} Chess_nll40.png nll30.bmp

${CONVERT} Chess_rdd40.png rdd30.bmp
${CONVERT} Chess_rdl40.png rdl30.bmp
${CONVERT} Chess_rld40.png rld30.bmp
${CONVERT} Chess_rll40.png rll30.bmp

${CONVERT} Chess_pdd40.png pdd30.bmp
${CONVERT} Chess_pdl40.png pdl30.bmp
${CONVERT} Chess_pld40.png pld30.bmp
${CONVERT} Chess_pll40.png pll30.bmp

