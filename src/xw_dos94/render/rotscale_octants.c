#include "xw/render/flight_view.h"
#include "xw_dos94/render/rotscale.h"

static uint16_t run_length(const Dos94Rotation* r, int index) {
	return index >= 0 && index < r->line->num_run_lengths ? r->line->run_lengths[index] : 0;
}

/* DOS94 0x6A3C5C: setstartcase0; corresponding OpenTIE ROTSCALE body. */
static int setstartcase0(Dos94Rotation* r) {
	int16_t plotx_loc = r->plotx;
	int16_t ploty_loc = r->ploty;
	int16_t row_off = 0;

	while (plotx_loc < 0) {
		ploty_loc = (int16_t)(ploty_loc + r->line->dy_abs + 1);
		plotx_loc = (int16_t)(plotx_loc + r->line->dx_abs + 1);
		row_off -= g_flightVpWidth;
	}
	while (plotx_loc >= g_flightVpWidth) {
		ploty_loc = (int16_t)(ploty_loc - (r->line->dy_abs + 1));
		plotx_loc = (int16_t)(plotx_loc - (r->line->dx_abs + 1));
		row_off += g_flightVpWidth;
	}
	int16_t y_off = (int16_t)r->line->dda[2 * plotx_loc + 1];
	int16_t lsy = (int16_t)(ploty_loc - y_off);
	if (lsy >= g_flightVpHeight) {
		r->linestarty = lsy;
		r->lineendy = (int16_t)(r->line->dy_abs + lsy);
		r->ploty = lsy;
		r->plotx = plotx_loc;
		return 0;
	}
	r->linestartx = 0;
	r->linestarty = lsy;
	int16_t first_vp = 0;
	if (lsy < 0) {
		int16_t neg = (int16_t)-lsy;
		r->firstyincoffset = neg;
		first_vp = -1;
		if (neg <= (int16_t)r->line->dy_abs) {
			for (unsigned i = 0; i <= r->line->dx_abs; ++i) {
				if (neg == (int16_t)r->line->dda[2 * i + 1]) {
					first_vp = (int16_t)r->line->dda[2 * i];
					break;
				}
			}
		}
	}
	r->lineendx = (int16_t)(r->line->dx_abs + r->linestartx);
	r->lastyincoffset = g_flightVpHeight;
	r->firstvispoint = first_vp;
	int16_t ley = (int16_t)(r->line->dy_abs + r->linestarty);
	r->lineendy = ley;
	int16_t last_vp;
	if (ley < 0) {
		last_vp = -1;
	} else if (ley >= g_flightVpHeight) {
		r->lastyincoffset = (int16_t)(g_flightVpHeight - r->linestarty);
		uint16_t k;
		for (k = 0; k <= r->line->dx_abs && (uint16_t)r->lastyincoffset != r->line->dda[2 * k + 1]; ++k)
			;
		last_vp = (int16_t)(r->line->dda[2 * k] - 1);
	} else {
		last_vp = (int16_t)(r->line->scan_count - 1);
	}
	r->lastvispoint = last_vp;
	r->startdrawpoint = (int16_t)(row_off + plotx_loc);
	r->ploty = lsy;
	r->plotx = plotx_loc;
	return 1;
}

/* DOS94 0x6A3DDD: updatecase0; corresponding OpenTIE ROTSCALE body. */
static int updatecase0(Dos94Rotation* r) {
	int16_t fvp = r->firstvispoint;
	int16_t fyi = r->firstyincoffset;
	int16_t ley_local = r->lineendy;
	int result;
	int16_t lsy_new = (int16_t)(r->linestarty + 1);
	if (r->linestarty == -1) {
		fvp = 0;
	} else if (lsy_new >= g_flightVpHeight) {

		result = 0;
		goto done;
	} else if (lsy_new < 0) {
		fyi = (int16_t)(r->firstyincoffset - 1);
		fvp = (int16_t)(r->firstvispoint - (int16_t)run_length(r, fyi));
	}
	ley_local = (int16_t)(r->lineendy + 1);
	if (r->lineendy == -1) {
		int16_t lyi = (int16_t)(r->line->num_run_lengths - 1);
		r->lastyincoffset = lyi;
		r->lastvispoint = (int16_t)(r->line->scan_count - 1);
		int16_t cand = (int16_t)(r->lastvispoint - (int16_t)run_length(r, lyi) + 1);
		if (cand < 0)
			cand = 0;
		fvp = cand;
	} else if (ley_local >= g_flightVpHeight) {
		if (ley_local == g_flightVpHeight)
			r->lastyincoffset = (int16_t)r->line->num_run_lengths;
		--r->lastyincoffset;
		r->lastvispoint = (int16_t)(r->lastvispoint - (int16_t)run_length(r, r->lastyincoffset));
	}
	result = 1;
done:
	r->firstyincoffset = fyi;
	++r->linestarty;
	r->lineendy = ley_local;
	r->firstvispoint = fvp;
	return result;
}

/* DOS94 0x6A3E65: setstartcase1; corresponding OpenTIE ROTSCALE body. */
static int setstartcase1(Dos94Rotation* r) {
	int16_t plotx_loc = r->plotx;
	int16_t ploty_loc = r->ploty;
	int16_t row_off = 0;
	while (plotx_loc < 0) {
		ploty_loc = (int16_t)(ploty_loc - (r->line->dy_abs + 1));
		plotx_loc = (int16_t)(plotx_loc + (r->line->dx_abs + 1));
		row_off -= g_flightVpWidth;
	}
	while (plotx_loc >= g_flightVpWidth) {
		ploty_loc = (int16_t)(ploty_loc + (r->line->dy_abs + 1));
		plotx_loc = (int16_t)(plotx_loc - (r->line->dx_abs + 1));
		row_off += g_flightVpWidth;
	}
	int16_t lsy = (int16_t)(r->line->dda[2 * plotx_loc + 1] + ploty_loc);
	r->linestarty = lsy;
	r->linestartx = 0;
	int16_t first_vp = -1;
	if (lsy >= 0) {
		if (lsy >= g_flightVpHeight) {
			r->firstyincoffset = (int16_t)(lsy - g_flightVpHeight);
			if ((int16_t)(lsy - g_flightVpHeight) + 1 > (int16_t)r->line->dx_abs) {
				first_vp = -1;
			} else {
				for (uint16_t i = 0; i <= r->line->dx_abs; ++i) {
					const uint16_t* e = &r->line->dda[2 * i];
					if ((uint16_t)(r->firstyincoffset + 1) == e[1]) {
						first_vp = (int16_t)e[0];
						break;
					}
				}
			}
		} else {
			first_vp = 0;
		}
	}
	r->lineendx = (int16_t)(r->line->dx_abs + r->linestartx);
	r->lastyincoffset = g_flightVpHeight;
	r->firstvispoint = first_vp;
	int16_t ley = (int16_t)(r->linestarty - r->line->dy_abs);
	r->lineendy = ley;
	if (ley >= g_flightVpHeight) {
		r->ploty = lsy;
		r->plotx = plotx_loc;
		return 0;
	}
	int16_t last_vp;
	if (ley < 0 || ley >= g_flightVpHeight) {
		if (r->linestarty < 0) {
			r->lastvispoint = -1;
			r->startdrawpoint = (int16_t)(row_off + plotx_loc);
			r->ploty = lsy;
			r->plotx = plotx_loc;
			return 1;
		}
		r->lastyincoffset = r->linestarty;
		uint16_t k;

		for (k = 0; k <= r->line->dx_abs &&
					(uint16_t)(r->line->dda[1] + r->linestarty + 1) != r->line->dda[2 * k + 1];
			 ++k)
			;
		last_vp = (int16_t)r->line->dda[2 * k];
	} else {
		last_vp = (int16_t)r->line->scan_count;
	}
	r->lastvispoint = (int16_t)(last_vp - 1);
	r->startdrawpoint = (int16_t)(row_off + plotx_loc);
	r->ploty = lsy;
	r->plotx = plotx_loc;
	return 1;
}

/* DOS94 0x6A3FBB: updatecase1; corresponding OpenTIE ROTSCALE body. */
static int updatecase1(Dos94Rotation* r) {
	int16_t lvp = r->lastvispoint;
	int16_t lsy_new = (int16_t)(r->linestarty + 1);
	if (r->linestarty == -1) {
		r->firstvispoint = 0;
		r->lastyincoffset = 0;

		lvp = (int16_t)(run_length(r, 0) - 1);
	} else if (lsy_new >= g_flightVpHeight) {
		if (lsy_new == g_flightVpHeight)
			r->firstyincoffset = -1;
		++r->firstyincoffset;

		r->firstvispoint = (int16_t)(r->firstvispoint + (int16_t)run_length(r, r->firstyincoffset));
	}
	int16_t ley_new = (int16_t)(r->lineendy + 1);
	if (r->lineendy == -1) {
		lvp = (int16_t)(r->line->scan_count - 1);
	} else if (ley_new >= g_flightVpHeight) {
		++r->lineendy;
		++r->linestarty;
		r->lastvispoint = lvp;
		return 0;
	} else if (ley_new < 0 && lsy_new > 0) {

		++r->lastyincoffset;
		lvp = (int16_t)(lvp + (int16_t)run_length(r, r->lastyincoffset));
	}
	++r->lineendy;
	++r->linestarty;
	r->lastvispoint = lvp;
	return 1;
}

/* DOS94 0x6A4056: setstartcase2; corresponding OpenTIE ROTSCALE body. */
static int setstartcase2(Dos94Rotation* r) {
	int16_t plotx_loc = r->plotx;
	int16_t ploty_loc = r->ploty;
	int16_t row_off = 0;
	while (plotx_loc < 0) {
		ploty_loc = (int16_t)(ploty_loc - (r->line->dy_abs + 1));
		plotx_loc = (int16_t)(plotx_loc + (r->line->dx_abs + 1));
		row_off += g_flightVpWidth;
	}
	while (plotx_loc >= g_flightVpWidth) {
		ploty_loc = (int16_t)(ploty_loc + (r->line->dy_abs + 1));
		plotx_loc = (int16_t)(plotx_loc - (r->line->dx_abs + 1));
		row_off -= g_flightVpWidth;
	}
	int16_t lsy = (int16_t)(ploty_loc - (int16_t)r->line->dda[2 * (g_flightVpMaxX - plotx_loc) + 1]);
	r->linestartx = g_flightVpMaxX;
	r->lineendx = (int16_t)(g_flightVpMaxX - r->line->dx_abs);
	r->lineendy = (int16_t)(r->line->dy_abs + lsy);
	r->linestarty = lsy;
	r->firstvispoint = -1;
	r->lastvispoint = -1;
	r->firstyincoffset = -1;
	r->lastyincoffset = g_flightVpHeight;
	int16_t first_vp = -1;
	int16_t x_pos = (int16_t)(g_flightVpMaxX - plotx_loc);
	if (lsy < 0) {
		r->firstyincoffset = (int16_t)(-lsy - 1);
		if ((int16_t)-lsy > (int16_t)r->line->dy_abs) {
			first_vp = -1;
		} else {
			for (int16_t i = 0; i <= (int16_t)r->line->dx_abs; ++i) {
				const uint16_t* e = &r->line->dda[2 * i];
				if ((uint16_t)-lsy == e[1]) {
					first_vp = (int16_t)e[0];
					break;
				}
			}
		}
	} else if (lsy >= g_flightVpHeight) {
		r->startdrawpoint = (int16_t)(row_off + x_pos);
		r->ploty = lsy;
		r->plotx = plotx_loc;
		return 1;
	} else {
		first_vp = 0;
	}
	r->firstvispoint = first_vp;
	int16_t last_vp = -1;
	if (r->lineendy >= 0) {
		if (r->lineendy >= g_flightVpHeight) {
			r->lastyincoffset = (int16_t)(g_flightVpHeight - r->linestarty - 1);
			int16_t k;
			for (k = (int16_t)r->line->dx_abs; k >= 0; --k) {
				const uint16_t* e = &r->line->dda[2 * k];
				if ((uint16_t)r->lastyincoffset == e[1]) {
					last_vp = (int16_t)e[0];
					break;
				}
			}
		} else {
			last_vp = (int16_t)(r->line->scan_count - 1);
		}
		r->lastvispoint = last_vp;
		r->startdrawpoint = (int16_t)(row_off + x_pos);
		r->ploty = lsy;
		r->plotx = plotx_loc;
		return 1;
	}
	r->ploty = lsy;
	r->plotx = plotx_loc;
	return 0;
}

/* DOS94 0x6A41DF: updatecase2; corresponding OpenTIE ROTSCALE body. */
static int updatecase2(Dos94Rotation* r) {
	int16_t lvp = r->lastvispoint;
	int16_t lsy_new = (int16_t)(r->linestarty - 1);
	if (lsy_new == g_flightVpMaxY) {
		r->firstvispoint = 0;
		r->firstyincoffset = -1;
		lvp = -1;
		r->lastyincoffset = -1;
	} else if (lsy_new < 0) {
		if (lsy_new == -1)
			r->firstvispoint = -1;
		++r->firstyincoffset;
		r->firstvispoint = (int16_t)(r->firstvispoint + (int16_t)run_length(r, r->firstyincoffset));
	}
	int16_t ley_new = (int16_t)(r->lineendy - 1);
	if (r->lineendy - 1 == g_flightVpMaxY) {
		lvp = (int16_t)(r->line->scan_count - 1);
	} else if (ley_new < 0) {
		--r->lineendy;
		--r->linestarty;
		r->lastvispoint = lvp;
		return 0;
	} else if (ley_new >= g_flightVpHeight && lsy_new < g_flightVpHeight) {
		++r->lastyincoffset;
		lvp = (int16_t)(lvp + (int16_t)run_length(r, r->lastyincoffset));
	}
	--r->lineendy;
	--r->linestarty;
	r->lastvispoint = lvp;
	return 1;
}

/* DOS94 0x6A427F: setstartcase3; corresponding OpenTIE ROTSCALE body. */
static int setstartcase3(Dos94Rotation* r) {
	int16_t plotx_loc = r->plotx;
	int16_t ploty_loc = r->ploty;
	int16_t row_off = 0;
	while (plotx_loc < 0) {
		ploty_loc = (int16_t)(ploty_loc + (r->line->dy_abs + 1));
		plotx_loc = (int16_t)(plotx_loc + (r->line->dx_abs + 1));
		row_off += g_flightVpWidth;
	}
	while (plotx_loc >= g_flightVpWidth) {
		ploty_loc = (int16_t)(ploty_loc - (r->line->dy_abs + 1));
		plotx_loc = (int16_t)(plotx_loc - (r->line->dx_abs + 1));
		row_off -= g_flightVpWidth;
	}
	int16_t lsy = (int16_t)(r->line->dda[2 * (g_flightVpMaxX - plotx_loc) + 1] + ploty_loc);
	r->linestartx = g_flightVpMaxX;
	r->lineendx = (int16_t)(g_flightVpMaxX - r->line->dx_abs);
	r->lineendy = (int16_t)(lsy - r->line->dy_abs);
	r->linestarty = lsy;
	r->firstvispoint = -1;
	r->lastvispoint = -1;
	r->firstyincoffset = -1;
	r->lastyincoffset = (int16_t)r->line->num_run_lengths;
	if (lsy < 0) {
		r->ploty = lsy;
		r->plotx = plotx_loc;
		return 0;
	}
	int16_t first_vp;
	if (lsy >= g_flightVpHeight) {
		r->firstyincoffset = (int16_t)(lsy - g_flightVpMaxY);
		if ((int16_t)(lsy - g_flightVpMaxY) > (int16_t)r->line->dy_abs) {
			first_vp = -1;
		} else {
			first_vp = -1;
			for (int16_t i = 0; i <= (int16_t)r->line->dx_abs; ++i) {
				const uint16_t* e = &r->line->dda[2 * i];
				if ((uint16_t)(lsy - g_flightVpMaxY) == e[1]) {
					first_vp = (int16_t)e[0];
					break;
				}
			}
		}
	} else {
		first_vp = 0;
	}
	r->firstvispoint = first_vp;
	int16_t last_vp;
	if (r->lineendy >= g_flightVpHeight) {
		last_vp = -1;
	} else if (r->lineendy < 0 || r->lineendy >= g_flightVpHeight) {
		int16_t off = (int16_t)(r->line->dy_abs + r->lineendy);
		r->lastyincoffset = (int16_t)(off + 1);
		int16_t k;
		last_vp = -1;
		for (k = (int16_t)r->line->dx_abs; k >= 0; --k) {
			const uint16_t* e = &r->line->dda[2 * k];
			if ((uint16_t)off == e[1]) {
				last_vp = (int16_t)e[0];
				break;
			}
		}
	} else {
		last_vp = (int16_t)(r->line->scan_count - 1);
	}
	r->lastvispoint = last_vp;
	r->startdrawpoint = (int16_t)(row_off + g_flightVpMaxX - plotx_loc);
	r->ploty = lsy;
	r->plotx = plotx_loc;
	return 1;
}

/* DOS94 0x6A4401: updatecase3; corresponding OpenTIE ROTSCALE body. */
static int updatecase3(Dos94Rotation* r) {
	int16_t fvp = r->firstvispoint;
	int16_t fyi = r->firstyincoffset;
	int16_t lyi = r->lastyincoffset;
	int16_t lsy_new = (int16_t)(r->linestarty - 1);
	int16_t ley_new = (int16_t)(r->lineendy - 1);
	if (lsy_new == g_flightVpMaxY) {
		fyi = -1;
		fvp = 0;
	} else {
		if (lsy_new < 0) {
			--r->linestarty;
			--r->lineendy;
			r->firstyincoffset = fyi;
			r->firstvispoint = fvp;
			return 0;
		}
		if (lsy_new >= g_flightVpHeight && ley_new < g_flightVpHeight) {
			fyi = (int16_t)(r->firstyincoffset - 1);
			fvp = (int16_t)(r->firstvispoint - (int16_t)run_length(r, fyi));
		}
	}
	if (ley_new == g_flightVpMaxY) {
		lyi = (int16_t)(r->line->num_run_lengths - 1);
		r->lastvispoint = (int16_t)(r->line->scan_count - 1);
		int16_t cand = (int16_t)(r->lastvispoint - (int16_t)run_length(r, lyi) + 1);
		if (cand < 0)
			cand = 0;
		fvp = cand;
	} else if (ley_new < 0) {
		lyi = (int16_t)(r->lastyincoffset - 1);
		r->lastvispoint = (int16_t)(r->lastvispoint - (int16_t)run_length(r, lyi));
	}
	--r->linestarty;
	--r->lineendy;
	r->lastyincoffset = lyi;
	r->firstyincoffset = fyi;
	r->firstvispoint = fvp;
	return 1;
}

/* DOS94 0x6A44A8: setstartcase4; corresponding OpenTIE ROTSCALE body. */
static int setstartcase4(Dos94Rotation* r) {
	int16_t ploty_loc = r->ploty;
	int16_t plotx_loc = r->plotx;
	int16_t row_off = 0;
	while (ploty_loc < 0) {
		ploty_loc = (int16_t)(ploty_loc + (r->line->dy_abs + 1));
		plotx_loc = (int16_t)(plotx_loc + (r->line->dx_abs + 1));
		row_off -= g_flightVpHeight;
	}
	while (ploty_loc >= g_flightVpHeight) {
		ploty_loc = (int16_t)(ploty_loc - (r->line->dy_abs + 1));
		plotx_loc = (int16_t)(plotx_loc - (r->line->dx_abs + 1));
		row_off += g_flightVpHeight;
	}
	int16_t lsx = (int16_t)(plotx_loc - (int16_t)r->line->dda[2 * ploty_loc]);
	r->linestarty = 0;
	r->firstvispoint = 0;
	r->firstxincoffset = -1;
	r->linestartx = lsx;
	r->lastxincoffset = g_flightVpWidth;
	if (lsx < 0) {
		r->firstxincoffset = (int16_t)(-lsx - 1);
		int16_t first_vp = -1;
		if ((int16_t)-lsx <= (int16_t)r->line->dx_abs) {
			for (int16_t i = 0; i <= (int16_t)r->line->dy_abs; ++i) {
				const uint16_t* e = &r->line->dda[2 * i];
				if ((uint16_t)-lsx == e[0]) {
					first_vp = (int16_t)e[1];
					break;
				}
			}
		}
		r->firstvispoint = first_vp;
	}
	r->lineendx = (int16_t)(r->line->dx_abs + r->linestartx);
	r->lineendy = (int16_t)(r->line->dy_abs + r->linestarty);
	if (r->lineendx < 0) {
		r->plotx = lsx;
		r->ploty = ploty_loc;
		return 0;
	}
	int16_t last_vp;
	if (r->lineendx >= g_flightVpWidth) {
		if (g_flightVpWidth <= r->linestartx) {
			last_vp = -1;
		} else {
			last_vp = -1;
			r->lastxincoffset = (int16_t)(g_flightVpWidth - r->linestartx - 1);
			for (int16_t k = 0; k <= (int16_t)r->line->dy_abs; ++k) {
				const uint16_t* e = &r->line->dda[2 * k];
				if ((uint16_t)(g_flightVpWidth - r->linestartx) == e[0]) {
					last_vp = (int16_t)(e[1] - 1);
					break;
				}
			}
		}
	} else {
		last_vp = (int16_t)(r->line->scan_count - 1);
	}
	r->lastvispoint = last_vp;
	r->startdrawpoint = (int16_t)(row_off + ploty_loc);
	r->plotx = lsx;
	r->ploty = ploty_loc;
	return 1;
}

/* DOS94 0x6A45C0: updatecase4; corresponding OpenTIE ROTSCALE body. */
static int updatecase4(Dos94Rotation* r) {
	int16_t lxi = r->lastxincoffset;
	int16_t lvp = r->lastvispoint;
	int16_t lsx_new = (int16_t)(r->linestartx - 1);
	int result;
	if (lsx_new == g_flightVpMaxX) {
		lvp = -1;
		r->firstvispoint = 0;
		lxi = -1;
	} else if (lsx_new < 0) {
		++r->firstxincoffset;
		r->firstvispoint = (int16_t)(r->firstvispoint + (int16_t)run_length(r, r->firstxincoffset));
	}
	int16_t lex_new = (int16_t)(r->lineendx - 1);
	if (lex_new < 0) {
		result = 0;
	} else {
		if (lex_new == g_flightVpMaxX) {
			lvp = (int16_t)(r->line->scan_count - 1);
		} else if (lex_new >= g_flightVpWidth && lsx_new < g_flightVpWidth) {
			++lxi;
			lvp = (int16_t)(lvp + (int16_t)run_length(r, lxi));
		}
		result = 1;
	}
	r->lastvispoint = lvp;
	r->lastxincoffset = lxi;
	--r->linestartx;
	--r->lineendx;
	return result;
}

/* DOS94 0x6A4644: setstartcase5; corresponding OpenTIE ROTSCALE body. */
static int setstartcase5(Dos94Rotation* r) {
	int16_t ploty_loc = r->ploty;
	int16_t plotx_loc = r->plotx;
	int16_t row_off = 0;
	while (ploty_loc < 0) {
		ploty_loc = (int16_t)(ploty_loc + (r->line->dy_abs + 1));
		plotx_loc = (int16_t)(plotx_loc - (r->line->dx_abs + 1));
		row_off += g_flightVpHeight;
	}
	while (ploty_loc >= g_flightVpHeight) {
		ploty_loc = (int16_t)(ploty_loc - (r->line->dy_abs + 1));
		plotx_loc = (int16_t)(plotx_loc + (r->line->dx_abs + 1));
		row_off -= g_flightVpHeight;
	}
	int16_t lsx = (int16_t)(plotx_loc - (int16_t)r->line->dda[2 * (g_flightVpMaxY - ploty_loc)]);
	r->linestarty = g_flightVpMaxY;
	r->lineendy = (int16_t)(g_flightVpMaxY - r->line->dy_abs);
	r->lineendx = (int16_t)(r->line->dx_abs + lsx);
	r->linestartx = lsx;
	r->firstvispoint = 0;
	r->firstxincoffset = -1;
	r->lastxincoffset = g_flightVpWidth;
	if (lsx >= g_flightVpWidth) {
		r->plotx = lsx;
		r->ploty = ploty_loc;
		return 0;
	}
	if (lsx < 0) {
		if (r->lineendx < 0) {
			r->lastvispoint = -1;
			r->startdrawpoint = (int16_t)(row_off + g_flightVpMaxY - ploty_loc);
			r->plotx = lsx;
			r->ploty = ploty_loc;
			return 1;
		}
		r->firstxincoffset = (int16_t)-lsx;
		int16_t first_vp = -1;
		if ((int16_t)-lsx <= (int16_t)r->line->dx_abs) {
			for (int16_t k = 0; k <= (int16_t)r->line->dy_abs; ++k) {
				const uint16_t* e = &r->line->dda[2 * k];
				if ((uint16_t)-lsx == e[0]) {
					first_vp = (int16_t)e[1];
					break;
				}
			}
		}
		r->firstvispoint = first_vp;
	}
	int16_t last_vp;
	if (r->lineendx >= g_flightVpWidth || r->lineendx < 0) {
		if (r->lineendx < g_flightVpWidth || g_flightVpWidth <= r->linestartx) {
			last_vp = -1;
		} else {
			r->lastxincoffset = (int16_t)(g_flightVpWidth - r->linestartx);
			last_vp = -1;
			for (int16_t k = 0; k <= (int16_t)r->line->dy_abs; ++k) {
				const uint16_t* e = &r->line->dda[2 * k];
				if ((uint16_t)(g_flightVpWidth - r->linestartx) == e[0]) {
					last_vp = (int16_t)(e[1] - 1);
					break;
				}
			}
		}
	} else {
		last_vp = (int16_t)(r->line->scan_count - 1);
	}
	r->lastvispoint = last_vp;
	r->startdrawpoint = (int16_t)(row_off + g_flightVpMaxY - ploty_loc);
	r->plotx = lsx;
	r->ploty = ploty_loc;
	return 1;
}

/* DOS94 0x6A4760: updatecase5; corresponding OpenTIE ROTSCALE body. */
static int updatecase5(Dos94Rotation* r) {
	int16_t fvp = r->firstvispoint;
	int16_t fxi = r->firstxincoffset;
	int16_t lex = r->lineendx;
	int16_t lsx_new = (int16_t)(r->linestartx + 1);
	if (r->linestartx == -1) {
		fvp = 0;
	} else if (lsx_new < 0) {
		fxi = (int16_t)(r->firstxincoffset - 1);

		if (fxi >= 0)
			fvp = (int16_t)(r->firstvispoint - (int16_t)run_length(r, fxi));
		if (fvp < 0)
			fvp = 0;
	}
	if ((lsx_new < 0 ? fvp : lsx_new) >= g_flightVpWidth) {

		++r->linestartx;
		r->firstxincoffset = fxi;
		r->firstvispoint = fvp;
		return 0;
	}
	int16_t lex_new = (int16_t)(lex + 1);
	if (lex == -1) {
		fxi = (int16_t)(r->line->num_run_lengths - 1);
		r->lastvispoint = (int16_t)(r->line->scan_count - 1);

		fvp = (int16_t)(g_flightVpHeight - (int16_t)run_length(r, fxi));
		if (fvp < 0)
			fvp = 0;
	} else if (lex_new >= g_flightVpWidth) {
		if (lex_new == g_flightVpWidth) {
			r->lastvispoint = g_flightVpMaxY;
			r->lastxincoffset = (int16_t)r->line->num_run_lengths;
		}
		--r->lastxincoffset;

		r->lastvispoint = (int16_t)(r->lastvispoint - (int16_t)run_length(r, r->lastxincoffset));
	}
	r->lineendx = lex_new;
	++r->linestartx;
	r->firstxincoffset = fxi;
	r->firstvispoint = fvp;
	return 1;
}

/* DOS94 0x6A480C: setstartcase6; corresponding OpenTIE ROTSCALE body. */
static int setstartcase6(Dos94Rotation* r) {
	int16_t ploty_loc = r->ploty;
	int16_t plotx_loc = r->plotx;
	int16_t row_off = 0;
	while (ploty_loc < 0) {
		ploty_loc = (int16_t)(ploty_loc + (r->line->dy_abs + 1));
		plotx_loc = (int16_t)(plotx_loc - (r->line->dx_abs + 1));
		row_off -= g_flightVpHeight;
	}
	while (ploty_loc >= g_flightVpHeight) {
		ploty_loc = (int16_t)(ploty_loc - (r->line->dy_abs + 1));
		plotx_loc = (int16_t)(plotx_loc + (r->line->dx_abs + 1));
		row_off += g_flightVpHeight;
	}
	int16_t lsx = (int16_t)(r->line->dda[2 * ploty_loc] + plotx_loc);
	r->linestarty = 0;
	r->lineendx = (int16_t)(lsx - r->line->dx_abs);
	r->lineendy = (int16_t)r->line->dy_abs;
	r->linestartx = lsx;
	r->firstvispoint = -1;
	r->firstxincoffset = -1;
	r->lastvispoint = -1;
	r->lastxincoffset = (int16_t)r->line->num_run_lengths;
	if (lsx < 0) {
		r->plotx = lsx;
		r->ploty = ploty_loc;
		return 0;
	}
	int16_t first_vp;
	if (lsx >= g_flightVpWidth) {
		r->firstxincoffset = (int16_t)(lsx - g_flightVpMaxX);
		if ((int16_t)(lsx - g_flightVpMaxX) > (int16_t)r->line->dx_abs) {
			first_vp = -1;
		} else {
			first_vp = -1;
			for (int16_t i = 0; i <= (int16_t)r->line->dy_abs; ++i) {
				const uint16_t* e = &r->line->dda[2 * i];
				if ((uint16_t)r->firstxincoffset == e[0]) {
					first_vp = (int16_t)e[1];
					break;
				}
			}
		}
	} else {
		first_vp = 0;
	}
	r->firstvispoint = first_vp;
	int16_t last_vp;
	if (r->lineendx >= g_flightVpWidth) {
		last_vp = -1;
	} else if (r->lineendx < 0 || r->lineendx >= g_flightVpWidth) {
		int16_t off = (int16_t)(r->line->dx_abs + r->lineendx);
		r->lastxincoffset = (int16_t)(off + 1);
		last_vp = -1;
		for (int16_t k = (int16_t)r->line->dy_abs; k >= 0; --k) {
			const uint16_t* e = &r->line->dda[2 * k];
			if ((uint16_t)off == e[0]) {
				last_vp = (int16_t)e[1];
				break;
			}
		}
	} else {
		last_vp = (int16_t)(r->line->scan_count - 1);
	}
	r->lastvispoint = last_vp;
	r->startdrawpoint = (int16_t)(row_off + ploty_loc);
	r->plotx = lsx;
	r->ploty = ploty_loc;
	return 1;
}

/* DOS94 0x6A4937: updatecase6; corresponding OpenTIE ROTSCALE body. */
static int updatecase6(Dos94Rotation* r) {
	int16_t fvp = r->firstvispoint;
	int16_t lvp = r->lastvispoint;
	int16_t fxi = r->firstxincoffset;
	int16_t lex_new = (int16_t)(r->lineendx - 1);
	int16_t lsx_new = (int16_t)(r->linestartx - 1);
	int result;
	if (lsx_new >= 0) {
		if (lsx_new == g_flightVpMaxX ||
			(lsx_new >= g_flightVpWidth && lex_new < g_flightVpWidth &&
			 (fxi = (int16_t)(r->firstxincoffset - 1),

			  fvp = (int16_t)(r->firstvispoint - (int16_t)run_length(r, fxi)), fvp < 0))) {
			fvp = 0;
		}
		if (lex_new == g_flightVpMaxX) {
			fxi = (int16_t)(r->line->num_run_lengths - 1);
			lvp = (int16_t)(r->line->scan_count - 1);

			fvp = (int16_t)(r->line->scan_count - (int16_t)run_length(r, fxi));
			if (fvp < 0)
				fvp = 0;
		} else if (lex_new < 0) {
			--r->lastxincoffset;

			lvp = (int16_t)(r->lastvispoint - (int16_t)run_length(r, r->lastxincoffset));
			if (lvp < 0)
				lvp = 0;
		}
		result = 1;
	} else {
		result = 0;
	}
	r->firstxincoffset = fxi;
	--r->lineendx;
	--r->linestartx;
	r->lastvispoint = lvp;
	r->firstvispoint = fvp;
	return result;
}

/* DOS94 0x6A49D1: setstartcase7; corresponding OpenTIE ROTSCALE body. */
static int setstartcase7(Dos94Rotation* r) {
	int16_t ploty_loc = r->ploty;
	int16_t plotx_loc = r->plotx;
	int16_t row_off = 0;
	while (ploty_loc < 0) {
		ploty_loc = (int16_t)(ploty_loc + (r->line->dy_abs + 1));
		plotx_loc = (int16_t)(plotx_loc + (r->line->dx_abs + 1));
		row_off += g_flightVpHeight;
	}
	while (ploty_loc >= g_flightVpHeight) {
		ploty_loc = (int16_t)(ploty_loc - (r->line->dy_abs + 1));
		plotx_loc = (int16_t)(plotx_loc - (r->line->dx_abs + 1));
		row_off -= g_flightVpHeight;
	}
	int16_t lsx = (int16_t)(r->line->dda[2 * (g_flightVpMaxY - ploty_loc)] + plotx_loc);
	r->linestarty = g_flightVpMaxY;
	r->lineendx = (int16_t)(lsx - r->line->dx_abs);
	r->lineendy = (int16_t)(g_flightVpMaxY - r->line->dy_abs);
	r->linestartx = lsx;
	r->firstxincoffset = -1;
	r->lastxincoffset = g_flightVpWidth;
	int16_t first_vp;
	if (lsx < 0) {
		first_vp = 0;
	} else if (lsx >= g_flightVpWidth) {
		r->firstxincoffset = (int16_t)(lsx - g_flightVpMaxX - 1);
		if ((int16_t)(lsx - g_flightVpMaxX) > (int16_t)r->line->dx_abs) {
			first_vp = -1;
		} else {
			first_vp = -1;
			for (int16_t i = 0; i <= (int16_t)r->line->dy_abs; ++i) {
				const uint16_t* e = &r->line->dda[2 * i];
				if ((uint16_t)(lsx - g_flightVpMaxX) == e[0]) {
					first_vp = (int16_t)e[1];
					break;
				}
			}
		}
	} else {
		first_vp = 0;
	}
	r->firstvispoint = first_vp;
	int16_t last_vp;
	if (r->lineendx >= g_flightVpWidth) {
		r->plotx = lsx;
		r->ploty = ploty_loc;
		return 0;
	}
	if (r->lineendx < 0 || r->lineendx >= g_flightVpWidth) {
		if (r->lineendx >= 0 || r->linestartx < 0) {
			last_vp = -1;
		} else {
			r->lastxincoffset = (int16_t)(r->line->dx_abs + r->lineendx);
			last_vp = -1;
			for (int16_t k = (int16_t)r->line->dy_abs; k >= 0; --k) {
				const uint16_t* e = &r->line->dda[2 * k];
				if ((uint16_t)r->lastxincoffset == e[0]) {
					last_vp = (int16_t)e[1];
					break;
				}
			}
		}
	} else {
		last_vp = (int16_t)(r->line->scan_count - 1);
	}
	r->lastvispoint = last_vp;
	r->startdrawpoint = (int16_t)(row_off + g_flightVpMaxY - ploty_loc);
	r->plotx = lsx;
	r->ploty = ploty_loc;
	return 1;
}

/* DOS94 0x6A4AF9: updatecase7; corresponding OpenTIE ROTSCALE body. */
static int updatecase7(Dos94Rotation* r) {
	int16_t lvp = r->lastvispoint;
	int16_t fxi = r->firstxincoffset;
	int16_t lsx_new = (int16_t)(r->linestartx + 1);
	if (r->linestartx == -1) {
		lvp = -1;
		r->firstvispoint = 0;
		r->lastxincoffset = -1;
	} else if (lsx_new >= g_flightVpWidth) {
		if (lsx_new == g_flightVpWidth)
			fxi = -1;
		++fxi;
		r->firstvispoint = (int16_t)(r->firstvispoint + (int16_t)run_length(r, fxi));
	}
	int16_t lex_new = (int16_t)(r->lineendx + 1);
	if (r->lineendx == -1) {
		lvp = (int16_t)(r->line->scan_count - 1);
	} else if (lex_new >= g_flightVpWidth) {
		++r->lineendx;
		++r->linestartx;
		r->firstxincoffset = fxi;
		r->lastvispoint = lvp;
		return 0;
	} else if (lex_new < 0 && lsx_new >= 0) {
		++r->lastxincoffset;
		lvp = (int16_t)(lvp + (int16_t)run_length(r, r->lastxincoffset));
	}
	r->firstxincoffset = fxi;
	r->lastvispoint = lvp;
	++r->lineendx;
	++r->linestartx;
	return 1;
}

bool Dos94Rot_Start(Dos94Rotation* r) {
	switch (r->line->octant_case) {
		case 0:
			return setstartcase0(r) != 0;
		case 1:
			return setstartcase1(r) != 0;
		case 2:
			return setstartcase2(r) != 0;
		case 3:
			return setstartcase3(r) != 0;
		case 4:
			return setstartcase4(r) != 0;
		case 5:
			return setstartcase5(r) != 0;
		case 6:
			return setstartcase6(r) != 0;
		case 7:
			return setstartcase7(r) != 0;
		default:
			return false;
	}
}

bool Dos94Rot_Step(Dos94Rotation* r) {
	switch (r->line->octant_case) {
		case 0:
			return updatecase0(r) != 0;
		case 1:
			return updatecase1(r) != 0;
		case 2:
			return updatecase2(r) != 0;
		case 3:
			return updatecase3(r) != 0;
		case 4:
			return updatecase4(r) != 0;
		case 5:
			return updatecase5(r) != 0;
		case 6:
			return updatecase6(r) != 0;
		case 7:
			return updatecase7(r) != 0;
		default:
			return false;
	}
}
