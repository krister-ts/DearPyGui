#pragma once

#include "mvTypeBases.h"

namespace Marvel {

	PyObject* unindent(PyObject* self, PyObject* args, PyObject* kwargs);

	MV_REGISTER_WIDGET(mvUnindent);
	class mvUnindent : public mvFloatPtrBase
	{

	public:

		static void InsertParser(std::map<std::string, mvPythonParser>* parsers);

	public:

		MV_APPITEM_TYPE(mvAppItemType::mvUnindent, "unindent")

		MV_START_GENERAL_CONSTANTS
		MV_END_GENERAL_CONSTANTS

		MV_START_COLOR_CONSTANTS
		MV_END_COLOR_CONSTANTS

		MV_START_STYLE_CONSTANTS
		MV_END_STYLE_CONSTANTS

		mvUnindent(const std::string& name, float default_value);

		void draw() override;

	};

}