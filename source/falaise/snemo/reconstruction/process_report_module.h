/// \file snemo/reconstruction/process_report_module.h
/* Author(s)     : Xavier Garrido <garrido@lal.in2p3.fr>
 * Creation date : 2015-12-04
 * Last modified : 2015-12-04
 *
 * Copyright (C) 2015 Xavier Garrido <garrido@lal.in2p3.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 *
 * Description:
 *
 * A module which goal is to produce a report of the data processing.
 *
 * History:
 *
 */

#ifndef FALAISE_CHARGEDPARTICLETRACKING_PLUGIN_RECONSTRUCTION_PROCESS_REPORT_MODULE_H
#define FALAISE_CHARGEDPARTICLETRACKING_PLUGIN_RECONSTRUCTION_PROCESS_REPORT_MODULE_H 1

// Third party:
// - Bayeux/dpp:
#include <dpp/base_module.h>

namespace geomtools {
  class manager;
}
namespace cuts {
  class cut_manager;
}

namespace snemo {

  namespace reconstruction {

    /// \brief A process report module
    class process_report_module : public dpp::base_module
    {
    public:

      /// Setting geometry manager
      void set_geometry_manager(const geomtools::manager & mgr_);

      /// Getting geometry manager
      const geomtools::manager & get_geometry_manager() const;

      /// Setting cut manager
      void set_cut_manager(const cuts::cut_manager & mgr_);

      /// Getting cut manager
      const cuts::cut_manager & get_cut_manager() const;

      /// Constructor
      process_report_module(datatools::logger::priority = datatools::logger::PRIO_FATAL);

      /// Destructor
      virtual ~process_report_module();

      /// Initialization
      virtual void initialize(const datatools::properties  & setup_,
                              datatools::service_manager   & service_manager_,
                              dpp::module_handle_dict_type & module_dict_);

      /// Reset
      virtual void reset();

      /// Data record processing
      virtual process_status process(datatools::things & data_);

      /// Give default values to specific class members.
      void _set_defaults();

    private:

      const geomtools::manager * _geometry_manager_; //!< The geometry manager
      const cuts::cut_manager * _cut_manager_;       //!< The cut manager

      // Macro to automate the registration of the module :
      DPP_MODULE_REGISTRATION_INTERFACE(process_report_module);

    };

  } // namespace reconstruction

} // namespace snemo

#include <datatools/ocd_macros.h>

// Declare the OCD interface of the module
DOCD_CLASS_DECLARATION(snemo::reconstruction::process_report_module)

#endif // FALAISE_CHARGEDPARTICLETRACKING_PLUGIN_RECONSTRUCTION_PROCESS_REPORT_MODULE_H

// end of snemo/reconstruction/process_report_module.h
/*
** Local Variables: --
** mode: c++ --
** c-file-style: "gnu" --
** tab-width: 2 --
** End: --
*/
