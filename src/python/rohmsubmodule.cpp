#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "rohm.h"


static PyObject *ROHM_ERROR;
static rohm::vehicle_params CUR_CAR;

static PyObject* rohm_estimate_path(PyObject* self, PyObject* args, PyObject* kwds)
{
	(void)self;
	unsigned int map_r, map_c;
	PyObject *trip_seq, *map_size;
	char* kw_names[] = {
		(char*)"trip",
		(char*)"size",
		(char*)"mass_kg",
		(char*)"avg_kwh_km",
		(char*)"regen_efficiency",
		(char*)"energy_kwh", 
		NULL,
	};

	PyArg_ParseTupleAndKeywords(
		args,
		kwds,
		"OOffff",
		kw_names,
		&trip_seq,
		&map_size,
		&CUR_CAR.mass_kg,
		&CUR_CAR.avg_kwh_km,
		&CUR_CAR.regen_efficiency,
		&CUR_CAR.energy_kwh
	);

	PyArg_ParseTuple(map_size, "II", &map_c, &map_r);

	// allocate the estimate map
	rohm::estimate_cell** map = new rohm::estimate_cell*[map_r];
	for (auto r = map_r; r--;)
	map[r] = new rohm::estimate_cell[map_c];

	// create c++ vector of coordinates from python sequence
	rohm::estimate_params params;
	if (PySequence_Check(trip_seq))
	{
		printf("trip is sequence\n");

		for (Py_ssize_t i = 0; i < PySequence_Length(trip_seq); i++)
		{
			double lat, lng, speed_km_h;
			auto item = PySequence_ITEM(trip_seq, i);
			PyArg_ParseTuple(item, "ddd", &lat, &lng, &speed_km_h);
			params.trip.waypoints.push_back({lat, lng});
			params.trip.avg_speed_km_h.push_back(speed_km_h);
			params.trip.ctx.push_back({});
		}
	}

	auto win = rohm::window_from_trip(params.trip);

	params.car = CUR_CAR;
	params.origin = params.trip.waypoints[0];

	rohm::estimate(map_r, map_c, map, params, true);

	rohm::write_tiff("/tmp/rohmxzy.tif", map_r, map_c, map, CUR_CAR);

	// create a python list containing SoC for the entire trip
	PyObject* estimated_trip = PyList_New(params.trip.waypoints.size());

	for (size_t i = 0; i < params.trip.waypoints.size(); i++)
	{
		size_t r, c;
		auto& coord = params.trip.waypoints[i];
		if (!coord_to_idx(map_c, map_r, win, coord, r, c)) { continue; }

		PyObject* waypoint = Py_BuildValue("(ddd)", coord.lat(), coord.lng(), map[r][c].energy_kwh / params.car.energy_kwh);
		PyList_SetItem(estimated_trip, i, waypoint);
	}

	// free the estimate map
	for (auto r = map_r; r--;) { delete map[r]; }
	delete[] map;

	return estimated_trip;
}


static PyObject* rohm_window_from_path(PyObject* self, PyObject* args)
{
	(void)self;
	rohm::trip trip;
	PyObject *trip_seq;
	PyArg_ParseTuple(args, "O", &trip_seq);

	if (PySequence_Check(trip_seq))
	{
		printf("trip is sequence\n");

		for (Py_ssize_t i = 0; i < PySequence_Length(trip_seq); i++)
		{
			double lat, lng, speed;
			auto item = PySequence_ITEM(trip_seq, i);
			PyArg_ParseTuple(item, "ddd", &lat, &lng, &speed);
			trip.waypoints.push_back({lat, lng});
		}
	}

	auto window = rohm::window_from_trip(trip);
	PyObject* nw = Py_BuildValue("(dd)", window.nw.lat(), window.nw.lng());
	PyObject* se = Py_BuildValue("(dd)", window.se.lat(), window.se.lng());

	return PyTuple_Pack(2, nw, se);
}


static PyMethodDef ROHM_METHODS[] = {
	{"estimate_path", (PyCFunction)rohm_estimate_path, METH_VARARGS|METH_KEYWORDS, "Compute energy consumption for a specific vehicle"},
	{"window_from_path", (PyCFunction)rohm_window_from_path, METH_VARARGS, "Compute GCS window for sequence of path points given"},
	{NULL, NULL, 0, NULL}        /* Sentinel */
};


static struct PyModuleDef rohmmodule = {
	PyModuleDef_HEAD_INIT,
	.m_name = "rohm",   /* name of module */
	.m_doc = NULL, /* module documentation, may be NULL */
	-1,       /* size of per-interpreter state of the module,
				 or -1 if the module keeps state in global variables. */
	.m_methods = ROHM_METHODS,
	.m_slots = NULL,
};


PyMODINIT_FUNC
PyInit_rohm(void)
{
	PyObject *m;

	m = PyModule_Create(&rohmmodule);
	if (m == NULL)
	{
		return NULL;
	}

	ROHM_ERROR = PyErr_NewException("rohm.error", NULL, NULL);
	Py_XINCREF(ROHM_ERROR);
	if (PyModule_AddObject(m, "error", ROHM_ERROR) < 0)
	{
		Py_XDECREF(ROHM_ERROR);
		Py_CLEAR(ROHM_ERROR);
		Py_DECREF(m);
		return NULL;
	}

	return m;
}
