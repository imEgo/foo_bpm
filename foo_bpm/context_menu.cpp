#include "stdafx.h"
#include "context_menu.h"
#include "guid.h"
#include "foo_bpm.h"
#include "bpm_auto_analysis_thread.h"

static contextmenu_group_popup_factory g_bpm_context_group(guid_bpm_context_group, contextmenu_groups::root, "BPM Analysis", 0);


GUID contextmenu_item_simple_bpm::get_parent()
{
	return guid_bpm_context_group;
}

unsigned contextmenu_item_simple_bpm::get_num_items()
{
	return mnuTotal;
}

void contextmenu_item_simple_bpm::get_item_name(unsigned p_index, pfc::string_base & p_out)
{
	switch (p_index)
	{
	    case mnuAutoAnalysis:
		    p_out = "Automatically analyse BPMs...";
			break;
		case mnuManualAnalysis:
			p_out = "Manually tap BPM for current track...";
			break;
		case mnuDoubleBPM:
			p_out = "Double selected BPMs";
			break;
		case mnuHalveBPM:
			p_out = "Halve selected BPMs";
			break;
		default:
			break;
	}
}

void contextmenu_item_simple_bpm::context_command(unsigned p_index, metadb_handle_list_cref p_data, const GUID& p_caller)
{
	switch (p_index)
	{
	    case mnuAutoAnalysis:
			run_auto_analysis(p_data);
			break;
		case mnuManualAnalysis:
			run_manual_analysis(p_data);
			break;
		case mnuDoubleBPM:
			run_double_or_halve_bpm(p_data, false);
			break;
		case mnuHalveBPM:
			run_double_or_halve_bpm(p_data, true);
			break;
		default:
			break;
	}
}

GUID contextmenu_item_simple_bpm::get_item_guid(unsigned p_index)
{
	switch (p_index)
	{
	    case mnuAutoAnalysis:
			return guid_auto_bpm;
			break;
		case mnuManualAnalysis:
			return guid_manual_bpm;
			break;
		case mnuDoubleBPM:
			return guid_double_bpm;
			break;
		case mnuHalveBPM:
			return guid_halve_bpm;
			break;
		default:
			break;
	}

	return pfc::guid_null;
}

bool contextmenu_item_simple_bpm::get_item_description(unsigned p_index, pfc::string_base & p_out)
{
	switch (p_index)
	{
	    case mnuAutoAnalysis:
	        p_out = "Perform an automatic BPM analysis of the selected tracks.";
			break;
		case mnuManualAnalysis:
			p_out = "Manually tap the BPM of the selected track.";
			break;
		case mnuDoubleBPM:
			p_out = "Double the BPM of the selected tracks.";
			break;
		case mnuHalveBPM:
			p_out = "Halve the BPM of the selected tracks.";
			break;
		default:
			return false;
			break;
	}

	return true;
}

void contextmenu_item_simple_bpm::run_auto_analysis(metadb_handle_list_cref p_data)
{
	service_ptr_t<bpm_auto_analysis_thread> thread = new service_impl_t<bpm_auto_analysis_thread>(p_data);
	thread->start();
}

void contextmenu_item_simple_bpm::run_manual_analysis(metadb_handle_list_cref p_data)
{
	bpm_manual_dialog* dlg = new bpm_manual_dialog();
	dlg->Create(core_api::get_main_window(), NULL);
	dlg->ShowWindow(SW_SHOWNORMAL);
}

void contextmenu_item_simple_bpm::run_double_or_halve_bpm(metadb_handle_list_cref p_data, bool p_halve)
{
	const pfc::string8 bpm_tag = bpm_config_bpm_tag;
	const int bpm_precision = bpm_config_bpm_precision;

	metadb_handle_list tracks;
	pfc::list_t<file_info_impl> infos;

	tracks.prealloc(p_data.get_size());
	infos.prealloc(p_data.get_size());

	// For each item in the playlist selection
	for (t_size index = 0; index < p_data.get_size(); index++)
	{
		metadb_handle_ptr track = p_data[index];
		file_info_impl info;

		if (track->get_info(info) && info.meta_exists(bpm_tag))
		{
			tracks.add_item(track);
			infos.add_item(info);
		}
	}

	char bpm_str[256];

	for (t_size index = 0; index < infos.get_size(); index++)
	{
		const char * str = infos[index].meta_get(bpm_tag, 0);

		float bpm = 0.0f;
		sscanf_s(str, "%f", &bpm);

		if (p_halve)
		{
			bpm = bpm / 2;
		}
		else
		{
			bpm = bpm * 2;
		}

		switch (bpm_precision)
		{
			case BPM_PRECISION_1:
				sprintf_s(bpm_str, "%d", (int)bpm);
				break;
			case BPM_PRECISION_1DP:
				sprintf_s(bpm_str, "%0.1f", bpm);
				break;
			case BPM_PRECISION_2DP:
				sprintf_s(bpm_str, "%0.2f", bpm);
				break;
		}

		infos[index].meta_set(bpm_tag, bpm_str);
	}

	static_api_ptr_t<metadb_io_v2>()->update_info_async_simple(tracks,
		pfc::ptr_list_const_array_t<const file_info, file_info_impl *>(infos.get_ptr(), infos.get_size()),
		core_api::get_main_window(),
		/* p_op_flags */ 0,
		/* p_notify */ NULL);
}

static contextmenu_item_factory_t<contextmenu_item_simple_bpm> contextmenu_factory;