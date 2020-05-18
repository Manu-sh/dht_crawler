#include "dht_crawler.h"

#include <fstream>
#include <iostream>

extern "C" {
	#include <unistd.h>
}

using std::cout, std::endl;

dht_crawler::dht_crawler(std::string result_file, int session_num, int start_port, int total_intervals) {
	m_total_intervals = total_intervals;
	m_session_num = session_num;
	m_start_port = start_port;
	m_result_file = result_file;
}

void dht_crawler::print_settings() const {

	cout << "\tupload_rate_limit: "      << m_upload_rate_limit      << endl;
	cout << "\tdownload_rate_limit: "    << m_download_rate_limit    << endl;
	cout << "\tactive_downloads: "       << m_active_downloads       << endl;
	cout << "\talert_queue_size: "       << m_alert_queue_size       << endl;
	cout << "\tdht_announce_interval: "  << m_dht_announce_interval  << endl;
	cout << "\ttorrent_upload_limit: "   << m_torrent_upload_limit   << endl;
	cout << "\ttorrent_download_limit: " << m_torrent_download_limit << endl;
	cout << "\tauto_manage_startup: "    << m_auto_manage_startup    << endl;
	cout << "\tauto_manage_interval: "   << m_auto_manage_interval   << endl;
	cout << "\tstart_port: "             << m_start_port             << endl;
	cout << "\tsession_num: "            << m_session_num            << endl;
	cout << "\ttotal_intervals: "        << m_total_intervals        << endl;
	cout << "\twriting_interval: "       << m_writing_interval       << endl;
	cout << "\tresult_file: "            << m_result_file            << endl;
	cout << "\ttrackers:" << endl;

	for (const auto &p : m_trackers)
		cout << "\t\t" << p.first << ":" << p.second << endl;
}

void dht_crawler::run() {

	cout << "Starting running with" << endl;
	this->print_settings();

	// sessione = client instance
	for (unsigned intervals = 0; intervals < m_total_intervals;) {

		// process data from every client
		for (unsigned i = 0; i < m_sessions.size(); ++i) {
			std::vector<lt::alert*> alerts;
			m_sessions.at(i)->pop_alerts(&alerts);
			m_sessions.at(i)->post_torrent_updates();
			this->handle_alerts(m_sessions.at(i), &alerts);
		}

		if (++intervals % m_writing_interval == 0) {

			cout << "interval " << intervals << " done, writing result file...";
			if (this->write_result_file())
				cout << " success." << endl;
			else
				cout << " failed." << endl;

			cout << "meta status: " << endl;
			cout << "\tcount: " << m_meta.size() << endl;
		}

		sleep(1);
	}

	cout << "Stopping running, writing result file..." << endl;
	if (this->write_result_file())
		cout << " success." << endl;
	else
		cout << " failed." << endl;

	// TODO: bug sure
	// remove any torrent after some seconds
	for (unsigned i = 0; i < m_sessions.size(); ++i) {

		auto torrents = m_sessions[i]->get_torrents();
		for (unsigned j = 0; j < torrents.size(); ++j)
			m_sessions[i]->remove_torrent(torrents[j]);

		// delete m_sessions[i]; 
	}
}

void dht_crawler::handle_alerts(libtorrent::session* psession, std::vector<libtorrent::alert*>* palerts) {

	while (palerts->size() > 0) {

		auto palert = palerts->front();
		palerts->pop_back();
		std::string info_hash;

		switch (palert->type()) {

			case libtorrent::dht_announce_alert::alert_type:
				{
					// get infohash here
					auto p2 = (libtorrent::dht_announce_alert*) palert;
					info_hash = p2->info_hash.to_string();

					// update meta
					// c++ map: if meta[info_hash] does not exist,
					// it will be inserted and initialized to zero

					// meta is a rank for certain infohash
					if (m_meta[info_hash] > 0)
					{
						++m_meta[info_hash];
					}
					else
					{
						m_meta[info_hash] = 1;
						++m_current_meta_count;
					}
					break;
				}
			case libtorrent::dht_get_peers_alert::alert_type:
				{

					// this
					auto p3 = (libtorrent::dht_get_peers_alert*) palert;
					info_hash = p3->info_hash.to_string();
					if (m_meta[info_hash] > 0)
					{
						++m_meta[info_hash];
					}
					else
					{
						// this
						m_info_hash_from_getpeers.push_back(info_hash); // add to a vector if is from getpeers (just as statistic)
						m_meta[info_hash] = 1;
						++m_current_meta_count;
					}
					break;
				}
			default:
				break;
		}

		// delete palert;
	}
}

void dht_crawler::create_sessions() {

	int &start_port = m_start_port;

	for (int i = 0; i < m_session_num; ++i) {

		auto psession = new libtorrent::session;
		// psession->set_alert_mask(libtorrent::alert::category_t::all_categories);

		lt::error_code ec;

		// ogni client ha la porta del precedente + 1
		psession->listen_on(std::make_pair(start_port + i, start_port + i), ec);

		// vengono aggiunti dei tracker di default alla istanza del client
		for (unsigned j = 0; j < m_trackers.size(); ++j)
			psession->add_dht_router(m_trackers[j]);

		// vengono aggiunte le impostazioni
		auto settings = psession->get_settings();
		namespace alert_type = lt::alert_category;
		settings.set_int(lt::settings_pack::int_types::alert_mask, lt::alert::all_categories);

		/*
		   settings.upload_rate_limit = m_upload_rate_limit;
		   settings.download_rate_limit = m_download_rate_limit;
		   settings.active_downloads = m_active_downloads;
		   settings.auto_manage_interval = m_auto_manage_interval;
		   settings.auto_manage_startup = m_auto_manage_startup;
		   settings.dht_announce_interval = m_dht_announce_interval;
		   settings.alert_queue_size = m_alert_queue_size;
		   psession->set_settings(settings);
		*/
		m_sessions.push_back(psession);

	}
}

bool dht_crawler::write_result_file() {

	std::fstream fs;
	fs.open(m_result_file, std::fstream::out|std::fstream::trunc);

	if (!fs.is_open())
		return false;

	// viene salvato l'hash insieme alla sua frequenza su un file
	for (auto iter = m_meta.begin(); iter != m_meta.end(); ++iter) {

		const std::string &s = iter->first;
		for (unsigned int i = 0; i < s.size(); ++i) {

			fs << std::hex;
			if (unsigned char c = s[i]; c < 16)
				fs << '0' << (unsigned int)c;
			else
				fs << (unsigned int)c;
		}

		fs << std::dec;
		fs << '\t' << iter->second << endl;
	}

	return true;
}
