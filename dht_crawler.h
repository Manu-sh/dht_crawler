// define BOOST_ASIO_SEPARATE_COMPILATION

#ifndef _DHT_CRAWLER_H
#define _DHT_CRAWLER_H

#include <libtorrent/entry.hpp>
#include <libtorrent/bencode.hpp>
#include <libtorrent/session.hpp>
#include <libtorrent/alert_types.hpp>
#include <libtorrent/session_settings.hpp>
#include <vector>
#include <map>

class dht_crawler
{
public:

	dht_crawler(std::string result_file, int session_num, int start_port, int total_intervals);
	void create_sessions();
	void print_settings() const;
	void run();

	// session settings
	int m_upload_rate_limit = 200000;
	int m_download_rate_limit = 200000;
	int m_active_downloads = 30;
	int m_alert_queue_size = 4000;
	int m_dht_announce_interval = 60;
	int m_torrent_upload_limit = 20000;
	int m_torrent_download_limit = 20000;
	int m_auto_manage_startup = 30;
	int m_auto_manage_interval = 15;

	// crawler settings
	int m_start_port = 32900;
	int m_session_num = 50;
	int m_total_intervals = 60;
	int m_writing_interval = 60;
	std::string m_result_file;

	// example: router.bittorrent.com
	std::vector< std::pair<std::string, int> > m_trackers = {
		{"router.bittorrent.com",  6881},
		{"router.utorrent.com",    6881},
		{"router.bitcomet.com",    6881},
		{"dht.transmissionbt.com", 6881}
	};

private:

	void handle_alerts(libtorrent::session*, std::vector<libtorrent::alert*>*);
	void add_magnet(std::string link);
	bool write_result_file();

	// storage
	int m_current_meta_count = 0;
	std::vector<libtorrent::session*> m_sessions;
	std::vector<std::string> m_info_hash_from_getpeers;
	std::map<std::string, int> m_meta;
};

#endif // _DHT_CRAWLER_H