/**
 * \file
 * \brief  Definition of the class Properties that implements a container of
 *         properties.
 *
 * \copyright
 * Copyright (c) 2012-2014, OpenGeoSys Community (http://www.opengeosys.org)
 *            Distributed under a Modified BSD License.
 *              See accompanying file LICENSE.txt or
 *              http://www.opengeosys.org/project/license
 *
 */

#ifndef PROPERTIES_H_
#define PROPERTIES_H_

#include <cstdlib>
#include <string>
#include <map>

#include <boost/any.hpp>
#include <boost/optional.hpp>

#include "logog/include/logog.hpp"

#include "Location.h"

#include "PropertyVector.h"

namespace MeshLib
{

/// @brief Material property manager on mesh items.
/// Class Properties manages scalar, vector or matrix properties. For instance
/// in groundwater flow porosity is scalar property and permeabilty can be
/// stored as a tensor property. Properties are assigned to mesh items, i.e.
/// Node or Element objects. The newProperty() method first creates a
/// PropertyVector of template type T (scalar, vector or matrix).
/// This class stores the PropertyVector, accessible by a combination of the
/// name and the type of the mesh item (Node or Element).
class Properties
{
public:
	/// Method creates a PropertyVector if a PropertyVector with the same name
	/// and the same type T was not already created before.
	/// There are two versions of this method. This method is used
	/// when every mesh item at hand has its own property value, i.e. \f$n\f$
	/// mesh item and \f$n\f$ different property values.
	/// The user has to ensure the correct usage of the vector later on.
	/// @tparam T type of the property value
	/// @param name the name of the property
	/// @param mesh_item_type for instance node or element assigned properties
	/// @return On success a reference to a PropertyVector packed into a
	///   boost::optional else an empty boost::optional.
	template <typename T>
	boost::optional<PropertyVector<T> &>
	newProperty(std::string const& name, MeshItemType mesh_item_type)
	{
		PropertyKeyType property_key(name, mesh_item_type);
		std::map<PropertyKeyType, boost::any>::const_iterator it(
			_properties.find(property_key)
		);
		if (it != _properties.end()) {
			WARN("A property of the name \"%s\" is already assigned to the mesh.",
				name.c_str());
			return boost::optional<PropertyVector<T> &>();
		}
		auto entry_info(
			_properties.insert(
				std::pair<PropertyKeyType, boost::any>(
					property_key, boost::any(PropertyVector<T>())
				)
			)
		);
		return boost::optional<PropertyVector<T> &>(
			boost::any_cast<PropertyVector<T> &>((entry_info.first)->second)
			);
	}

	/// Method creates a PropertyVector if a PropertyVector with the same name
	/// and the same type T was not already created before.
	/// This method is used if only a small number of different properties
	/// exists. In this case a mapping between mesh items and properties (stored
	/// on the heap), see the parameter item2group_mapping, is required.
	/// @tparam T type of the property value
	/// @param name the name of the property
	/// @param mesh_item_type for instance node or element assigned properties
	/// @return On success a reference to a PropertyVector packed into a
	///   boost::optional else an empty boost::optional.
	template <typename T>
	boost::optional<PropertyVector<T> &>
	newProperty(std::string const& name,
		std::size_t n_prop_groups,
		std::vector<std::size_t> const& item2group_mapping,
		MeshItemType mesh_item_type)
	{
		PropertyKeyType property_key(name, mesh_item_type);
		std::map<PropertyKeyType, boost::any>::const_iterator it(
			_properties.find(property_key)
		);
		if (it != _properties.end()) {
			WARN("A property of the name \"%s\" already assigned to the mesh.",
				name.c_str());
			return boost::optional<PropertyVector<T> &>();
		}
		auto entry_info(
			_properties.insert(
				std::pair<PropertyKeyType, boost::any>(
					property_key,
					boost::any(PropertyVector<T>(n_prop_groups, item2group_mapping))
				)
			)
		);
		return boost::optional<PropertyVector<T> &>(
			boost::any_cast<PropertyVector<T> &>(
				(entry_info.first)->second)
			);
	}

	/// Method to get a vector of property values.
	template <typename T>
	boost::optional<PropertyVector<T> const&>
	getProperty(std::string const& name,
		MeshItemType mesh_item_type)
	{
		PropertyKeyType property_key(name, mesh_item_type);
		std::map<PropertyKeyType, boost::any>::const_iterator it(
			_properties.find(property_key)
		);
		if (it != _properties.end()) {
			try {
				return boost::optional<PropertyVector<T> const&>(
						boost::any_cast<PropertyVector<T> const&>(it->second)
					);
			} catch (boost::bad_any_cast const&) {
				ERR("A property with the desired data type is not available.");
				return boost::optional<PropertyVector<T> const&>();
			}
		} else {
			return boost::optional<PropertyVector<T> const&>();
		}
	}

	void removeProperty(std::string const& name,
		MeshItemType mesh_item_type)
	{
		PropertyKeyType property_key(name, mesh_item_type);
		std::map<PropertyKeyType, boost::any>::const_iterator it(
			_properties.find(property_key)
		);
		if (it == _properties.end()) {
			WARN("A property of the name \"%s\" does not exist.",
				name.c_str());
			return;
		}
		_properties.erase(it);
	}

	/// Check if a PropertyVector accessible by a combination of the
	/// name and the type of the mesh item (Node or Element) is already
	/// stored within the Properties object.
	/// @param name the name of the property (for instance porosity)
	/// @param mesh_item_type to which item type the property is assigned to
	bool hasProperty(std::string const& name, MeshItemType mesh_item_type)
	{
		PropertyKeyType property_key(name, mesh_item_type);
		std::map<PropertyKeyType, boost::any>::const_iterator it(
			_properties.find(property_key)
		);
		if (it == _properties.end()) {
			return false;
		}
		return true;
	}

private:
	struct PropertyKeyType
	{
		PropertyKeyType(std::string const& n, MeshItemType t)
			: name(n), mesh_item_type(t)
		{}

		std::string name;
		MeshItemType mesh_item_type;

		bool operator<(PropertyKeyType const& other) const
		{
			if (name.compare(other.name) == 0) {
				return mesh_item_type < other.mesh_item_type;
			}
			return name.compare(other.name) < 0;
		}
	};

	/// A mapping from property's name to the stored object of any type.
	/// See addProperty() and getProperty() documentation.
	std::map<PropertyKeyType, boost::any> _properties;
}; // end class

} // end namespace MeshLib

#endif

