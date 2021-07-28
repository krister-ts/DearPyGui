#include "mvDrawPolygon.h"
#include "mvLog.h"
#include "mvItemRegistry.h"
#include "mvPythonExceptions.h"

namespace Marvel {

	void mvDrawPolygon::InsertParser(std::map<std::string, mvPythonParser>* parsers)
	{

		mvPythonParser parser(mvPyDataType::UUID, "Draws a polygon on a drawing. First and and last point should be the same to close teh polygone.", { "Drawlist", "Widgets" });
		mvAppItem::AddCommonArgs(parser, (CommonParserArgs)(
			MV_PARSER_ARG_ID |
			MV_PARSER_ARG_PARENT |
			MV_PARSER_ARG_BEFORE |
			MV_PARSER_ARG_SHOW)
		);

		parser.addArg<mvPyDataType::ListFloatList>("points");

		parser.addArg<mvPyDataType::IntList>("color", mvArgType::KEYWORD_ARG, "(255, 255, 255, 255)");
		parser.addArg<mvPyDataType::IntList>("fill", mvArgType::KEYWORD_ARG, "(0, 0, 0, -255)");

		parser.addArg<mvPyDataType::Float>("thickness", mvArgType::KEYWORD_ARG, "1.0");

		parser.finalize();

		parsers->insert({ s_command, parser });
	}

	mvDrawPolygon::mvDrawPolygon(mvUUID uuid)
		:
		mvAppItem(uuid)
	{
	}

	bool mvDrawPolygon::isParentCompatible(mvAppItemType type)
	{
		if (type == mvAppItemType::mvStagingContainer) return true;
		if (type == mvAppItemType::mvDrawlist) return true;
		if (type == mvAppItemType::mvWindowAppItem) return true;
		if (type == mvAppItemType::mvPlot) return true;
		if (type == mvAppItemType::mvDrawLayer) return true;
		if (type == mvAppItemType::mvViewportDrawlist) return true;

		mvThrowPythonError(mvErrorCode::mvIncompatibleParent, s_command,
			"Incompatible parent. Acceptable parents include: staging container, drawlist, layer, window, plot, viewport drawlist.", this);

		MV_ITEM_REGISTRY_ERROR("Drawing item parent must be a drawing.");
		assert(false);
		return false;
	}

	void mvDrawPolygon::draw(ImDrawList* drawlist, float x, float y)
	{
		mvVec2 start = { x, y };
		std::vector<mvVec2> points = _points;
		if (ImPlot::GetCurrentContext()->CurrentPlot)
		{
			for (auto& point : points)
			{
				ImVec2 impoint = ImPlot::PlotToPixels(point);
				point.x = impoint.x;
				point.y = impoint.y;
			}
		}
		else
		{
			for (auto& point : points)
				point = point + start;
		}
		// TODO: Find a way to store lines and only calc new fill lines when dirty similar to ellipse
		drawlist->AddPolyline((const ImVec2*)const_cast<const mvVec2*>(points.data()), (int)_points.size(), _color, false, _thickness);
		if (_fill.r < 0.0f)
			return;

		{
			size_t i;
			int y;
			int miny, maxy;
			int x1, y1;
			int x2, y2;
			int ind1, ind2;
			size_t ints;
			size_t n = _points.size();
			int* polyints = new int[n];

			/* Determine Y maxima */
			miny = (int)_points[0].y;
			maxy = (int)_points[0].y;
			for (i = 1; i < n; i++)
			{
				miny = std::min(miny, (int)_points[i].y);
				maxy = std::max(maxy, (int)_points[i].y);
			}

			/* Draw, scanning y */
			for (y = miny; y <= maxy; y++) {
				ints = 0;
				for (i = 0; (i < n); i++) {
					if (!i)
					{
						ind1 = (int)n - 1;
						ind2 = 0;
					}
					else
					{
						ind1 = (int)i - 1;
						ind2 = (int)i;
					}
					y1 = (int)_points[ind1].y;
					y2 = (int)_points[ind2].y;
					if (y1 < y2)
					{
						x1 = (int)_points[ind1].x;
						x2 = (int)_points[ind2].x;
					}
					else if (y1 > y2)
					{
						y2 = (int)_points[ind1].y;
						y1 = (int)_points[ind2].y;
						x2 = (int)_points[ind1].x;
						x1 = (int)_points[ind2].x;
					}
					else
						continue;

					if (((y >= y1) && (y < y2)) || ((y == maxy) && (y > y1) && (y <= y2)))
						polyints[ints++] = (y - y1) * (x2 - x1) / (y2 - y1) + x1;

				}

				auto compare_int = [](const void* a, const void* b)
				{
					return (*(const int*)a) - (*(const int*)b);
				};

				qsort(polyints, ints, sizeof(int), compare_int);

				for (i = 0; i < ints; i += 2)
				{
					if (ImPlot::GetCurrentContext()->CurrentPlot)
						drawlist->AddLine(ImPlot::PlotToPixels({ (float)polyints[i], (float)y }),
							ImPlot::PlotToPixels({ (float)polyints[i + 1], (float)y}), _fill, ImPlot::GetCurrentContext()->Mx * _thickness);
					else
					{
						drawlist->AddLine({ (float)polyints[i] + start.x, (float)y + start.y },
							{ (float)polyints[i + 1] + start.x, (float)y + start.y }, _fill, _thickness);
					}
				}
			}
			delete[] polyints;
		}
	}

	void mvDrawPolygon::handleSpecificRequiredArgs(PyObject* dict)
	{
		if (!mvApp::GetApp()->getParsers()[s_command].verifyRequiredArguments(dict))
			return;

		for (int i = 0; i < PyTuple_Size(dict); i++)
		{
			PyObject* item = PyTuple_GetItem(dict, i);
			switch (i)
			{
			case 0:
				_points = ToVectVec2(item);
				break;

			default:
				break;
			}
		}
	}

	void mvDrawPolygon::handleSpecificKeywordArgs(PyObject* dict)
	{
		if (dict == nullptr)
			return;

		if (PyObject* item = PyDict_GetItemString(dict, "points")) _points = ToVectVec2(item);
		if (PyObject* item = PyDict_GetItemString(dict, "fill")) _fill = ToColor(item);
		if (PyObject* item = PyDict_GetItemString(dict, "color")) _color = ToColor(item);
		if (PyObject* item = PyDict_GetItemString(dict, "thickness")) _thickness = ToFloat(item);

	}

	void mvDrawPolygon::getSpecificConfiguration(PyObject* dict)
	{
		if (dict == nullptr)
			return;

		PyDict_SetItemString(dict, "points", ToPyList(_points));
		PyDict_SetItemString(dict, "fill", ToPyColor(_fill));
		PyDict_SetItemString(dict, "color", ToPyColor(_color));
		PyDict_SetItemString(dict, "thickness", ToPyFloat(_thickness));

	}

}