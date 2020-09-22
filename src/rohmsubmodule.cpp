#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "rohm.h"

static PyObject *ROHM_ERROR;
static rohm::vehicle_params CUR_CAR;

static PyObject* rohm_estimate_path(PyObject* self, PyObject* args, PyObject* kwds)
{
	PyObject* trip;
	char* kw_names[] = {
		"trip",
		"mass_kg",
		"avg_kwh_km",
		"regen_efficiency",
		"energy_kwh",
		NULL
	};

	PyArg_ParseTupleAndKeywords(
		args,
		kwds,
		"Offff",
		kw_names,
		&trip,
		&CUR_CAR.mass_kg,
		&CUR_CAR.avg_kwh_km,
		&CUR_CAR.regen_efficiency,
		&CUR_CAR.energy_kwh
	);

	printf("car mass: %f\n", CUR_CAR.mass_kg);
	printf("car avg eff: %f\n", CUR_CAR.avg_kwh_km);
	printf("car regen eff: %f\n", CUR_CAR.regen_efficiency);
	printf("car energy: %f\n", CUR_CAR.energy_kwh);

	return PyLong_FromLong(1);
}


static PyMethodDef ROHM_METHODS[] = {
	{"estimate_path", (PyCFunction)rohm_estimate_path, METH_VARARGS|METH_KEYWORDS, "Compute energy consumption for a specific vehicle"},
	{NULL, NULL, 0, NULL}        /* Sentinel */
};


static struct PyModuleDef rohmmodule = {
	PyModuleDef_HEAD_INIT,
	"rohm",   /* name of module */
	NULL, /* module documentation, may be NULL */
	-1,       /* size of per-interpreter state of the module,
				 or -1 if the module keeps state in global variables. */
	ROHM_METHODS
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