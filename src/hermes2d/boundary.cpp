// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#include "boundary.h"
#include "module.h"
#include "module_agros.h"
#include "scene.h"
#include "util.h"

Boundary::Boundary(std::string name, std::string type,
                   std::map<std::string, Value> values)
{
    // name and type
    this->name = name;
    this->type = type;
    this->values = values;

    // set values
    if (name != "none")
    {
        if (this->values.size() == 0)
        {
            Hermes::Module::BoundaryType *boundary_type = Util::scene()->fieldInfo("TODO")->module()->get_boundary_type(type);
            for (Hermes::vector<Hermes::Module::BoundaryTypeVariable *>::iterator it = boundary_type->variables.begin(); it < boundary_type->variables.end(); ++it)
            {
                Hermes::Module::BoundaryTypeVariable *variable = ((Hermes::Module::BoundaryTypeVariable *) *it);
                this->values[variable->id] = Value(QString::number(variable->default_value));
            }
        }
    }
}

Boundary::~Boundary()
{
    values.clear();
}

const Value Boundary::getValue(std::string id)
{
    if (id != "")
        return values[id];

    return Value();
}
