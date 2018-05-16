//
//  wrapper.cpp
//  iTorrent
//
//  Created by  XITRIX on 12.05.2018.
//  Copyright © 2018  XITRIX. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <list>
#include <boost/make_shared.hpp>
#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/torrent_info.hpp"
#include "libtorrent/announce_entry.hpp"
#include "libtorrent/magnet_uri.hpp"
#include "libtorrent/torrent_status.hpp"
#include "libtorrent/alert_types.hpp"
#include "result_struct.h"
#include "file_struct.h"

extern "C" int load_file(std::string const& filename, std::vector<char>& v
              , libtorrent::error_code& ec, int limit = 8000000)
{
    ec.clear();
    FILE* f = fopen(filename.c_str(), "rb");
    if (f == NULL)
    {
        ec.assign(errno, boost::system::system_category());
        fprintf(stderr, "err 1\n");
        return -1;
    }
    
    int r = fseek(f, 0, SEEK_END);
    if (r != 0)
    {
        ec.assign(errno, boost::system::system_category());
        fclose(f);
        fprintf(stderr, "err 2\n");
        return -1;
    }
    long s = ftell(f);
    if (s < 0)
    {
        ec.assign(errno, boost::system::system_category());
        fclose(f);
        fprintf(stderr, "err 3\n");
        return -1;
    }
    
    if (s > limit)
    {
        fclose(f);
        fprintf(stderr, "err 4\n");
        return -2;
    }
    
    r = fseek(f, 0, SEEK_SET);
    if (r != 0)
    {
        ec.assign(errno, boost::system::system_category());
        fclose(f);
        fprintf(stderr, "err 5\n");
        return -1;
    }
    
    v.resize(s);
    if (s == 0)
    {
        fclose(f);
        return 0;
    }
    
    r = fread(&v[0], 1, v.size(), f);
    if (r < 0)
    {
        ec.assign(errno, boost::system::system_category());
        fclose(f);
        return -1;
    }
    
    fclose(f);
    
    if (r != s) return -3;
    
    return 0;
}

extern "C" int print_torrent_info(libtorrent::torrent_info t) {
    using namespace libtorrent;
    namespace lt = libtorrent;
    
    // print info about torrent
    printf("\n\n----- torrent file info -----\n\n"
           "nodes:\n");
    
    typedef std::vector<std::pair<std::string, int> > node_vec;
    node_vec const& nodes = t.nodes();
    for (node_vec::const_iterator i = nodes.begin(), end(nodes.end());
         i != end; ++i)
    {
        printf("%s: %d\n", i->first.c_str(), i->second);
    }
    puts("trackers:\n");
    for (std::vector<announce_entry>::const_iterator i = t.trackers().begin();
         i != t.trackers().end(); ++i)
    {
        printf("%2d: %s\n", i->tier, i->url.c_str());
    }
    
    char ih[41];
    to_hex((char const*)&t.info_hash()[0], 20, ih);
    printf("number of pieces: %d\n"
           "piece length: %d\n"
           "info hash: %s\n"
           "comment: %s\n"
           "created by: %s\n"
           "magnet link: %s\n"
           "name: %s\n"
           "number of files: %d\n"
           "files:\n"
           , t.num_pieces()
           , t.piece_length()
           , ih
           , t.comment().c_str()
           , t.creator().c_str()
           , make_magnet_uri(t).c_str()
           , t.name().c_str()
           , t.num_files());
    file_storage const& st = t.files();
    for (int i = 0; i < st.num_files(); ++i)
    {
        printf("%s\n"
               , st.file_path(i).c_str());
        
//        int first = st.map_file(i, 0, 0).piece;
//        int last = st.map_file(i, (std::max)(boost::int64_t(st.file_size(i))-1, boost::int64_t(0)), 0).piece;
//        int flags = st.file_flags(i);
//
//        printf(" %8" PRIx64 " %11" PRId64 " %c%c%c%c [ %5d, %5d ] %7u %s %s %s%s\n"
//               , st.file_offset(i)
//               , st.file_size(i)
//               , ((flags & file_storage::flag_pad_file)?'p':'-')
//               , ((flags & file_storage::flag_executable)?'x':'-')
//               , ((flags & file_storage::flag_hidden)?'h':'-')
//               , ((flags & file_storage::flag_symlink)?'l':'-')
//               , first, last
//               , boost::uint32_t(st.mtime(i))
//               , st.hash(i) != sha1_hash(0) ? to_hex(st.hash(i).to_string()).c_str() : ""
//               , st.file_path(i).c_str()
//               , (flags & file_storage::flag_symlink) ? "-> " : ""
//               , (flags & file_storage::flag_symlink) ? st.symlink(i).c_str() : "");
    }
    return 0;
}

libtorrent::torrent_info* get_torrent(std::string file_path) {
    using namespace libtorrent;
    namespace lt = libtorrent;
    
    printf("%s\n", file_path.c_str());
    
    int item_limit = 1000000;
    int depth_limit = 1000;
    
    std::vector<char> buf;
    error_code ec;
    int ret = load_file(file_path, buf, ec, 40 * 1000000);
    if (ret == -1)
    {
        fprintf(stderr, "file too big, aborting\n");
        return NULL;
    }
    
    if (ret != 0)
    {
        fprintf(stderr, "failed to load file: %s\n", ec.message().c_str());
        return NULL;
    }
    bdecode_node e;
    int pos = -1;
    printf("decoding. recursion limit: %d total item count limit: %d\n"
           , depth_limit, item_limit);
    ret = bdecode(&buf[0], &buf[0] + buf.size(), e, ec, &pos
                  , depth_limit, item_limit);
    
    //printf("\n\n----- raw info -----\n\n%s\n", print_entry(e).c_str());
    
    if (ret != 0)
    {
        fprintf(stderr, "failed to decode: '%s' at character: %d\n", ec.message().c_str(), pos);
        return NULL;
    }
    
    return new torrent_info(e, ec);
}

using namespace libtorrent;
using namespace std;
class Engine {
public:
    static Engine *standart;
    session *s;
    std::string root_path;
    vector<torrent_handle> handlers;
    
    Engine(std::string root_path) {
        Engine::standart = this;
        this->root_path = root_path;
        
        settings_pack sett;
        sett.set_str(settings_pack::listen_interfaces, "0.0.0.0:6881");
        s = new lt::session(sett);
        
        
        std::ifstream in((Engine::standart->root_path + "/_Config/state.fastresume").c_str(), std::ios_base::binary);
        std::vector<char> buffer;
        
        if (!in.eof() && !in.fail())
        {
            in.seekg(0, std::ios_base::end);
            std::streampos fileSize = in.tellg();
            buffer.resize(fileSize);
            
            in.seekg(0, std::ios_base::beg);
            in.read(&buffer[0], fileSize);
        }
        
        lt::entry save = bdecode(buffer.begin(), buffer.end());
        s->save_state(save);
    }
    
    void addTorrent(torrent_info *torrent) {
        lt::error_code ec;
        add_torrent_params p;
        p.save_path = root_path;
        p.ti = boost::shared_ptr<torrent_info>(torrent);
        torrent_handle h = s->add_torrent(p, ec);
        
        handlers.push_back(h);
    }
    
    void addTorrentWithStates(torrent_info *torrent, int states[]) {
        lt::error_code ec;
        add_torrent_params p;
        p.save_path = root_path;
        p.ti = boost::shared_ptr<torrent_info>(torrent);
        torrent_handle handle = s->add_torrent(p, ec);
        handlers.push_back(handle);
        for (int i = 0; i < torrent->num_files(); i++) {
            handle.file_priority(i, states[i] ? 4 : 0);
        }
    }
    
    void addMagnet(char* magnetLink) {
        lt::error_code ec;
        add_torrent_params p;
        p.save_path = root_path;
        p.url = std::string(magnetLink);
        handlers.push_back(s->add_torrent(p, ec));
    }
    
    torrent_handle* getHandleByHash(char* torrent_hash) {
        //strcspn(torrent_hash, "\r\n");
        for (int i = 0; i < handlers.size(); i++) {
            //char* hash = new char[handlers[i].info_hash().get_hash_string().length() + 1];
            //strcpy(hash, handlers[i].info_hash().get_hash_string().c_str());
            //strcspn(hash, "\r\n");
            //printf("%s : size - %d\n", hash, strlen(handlers[i].info_hash().data()));
            if (strcmp(handlers[i].status().hash_to_string().c_str(), torrent_hash) == 0) {
                //printf("%s --- %s\n", handlers[i].info_hash().get_hash_string().c_str(), hash);
                //printf("FOUND!: %s", handlers[i].name().c_str());
                return &(handlers[i]);
            }
        }
        return NULL;
    }
    
    void printAllHandles() {
        for(int i = 0; i < handlers.size(); i++) {
            torrent_status stat = handlers[i].status();
            
            std::string state_str[] = {"queued", "checking", "downloading metadata", "downloading", "finished", "seeding", "allocating", "checking fastresume"};
            printf("\r%.2f%% complete (down: %.1d kb/s up: %.1d kB/s peers: %d) %s", stat.progress * 100, stat.download_rate / 1000, stat.upload_rate / 1000, stat.num_peers, state_str[stat.state].c_str());
        }
    }
};

Engine *Engine::standart = NULL;
extern "C" int init_engine(char* save_path) {
    new Engine(save_path);
    return 0;
}

extern "C" void add_torrent(char* torrent_path) {
    Engine::standart->addTorrent(get_torrent(torrent_path));
}

extern "C" void add_torrent_with_states(char* torrent_path, int states[]) {
    Engine::standart->addTorrentWithStates(get_torrent(torrent_path), states);
}

extern "C" void add_magnet(char* magnet_link) {
    Engine::standart->addMagnet(magnet_link);
}

extern "C" Files get_files_of_torrent(torrent_info* info) {
    file_storage fs = info->files();
    Files files {
        .size = fs.num_files(),
        .file_name = new char*[fs.num_files()],
        .file_size = new long long[fs.num_files()],
        .file_priority = new int[fs.num_files()]
    };
    files.title = new char[info->name().length() + 1];
    strcpy(files.title, info->name().c_str());
    for (int i = 0; i < fs.num_files(); i++) {
        files.file_name[i] = new char[fs.file_path(i).length() + 1];
        strcpy(files.file_name[i], fs.file_path(i).c_str());
        
        files.file_size[i] = fs.file_size(i);
    }
    return files;
}

extern "C" void set_torrent_files_priority(char* torrent_hash, int states[]) {
    torrent_handle* handle = Engine::standart->getHandleByHash(torrent_hash);
    torrent_info* info = (torrent_info*)&handle->get_torrent_info();
    for (int i = 0; i < info->num_files(); i++) {
        handle->file_priority(i, states[i]);
    }
//    printf("SETTED! %d\n", states[0]);
//    printf("%d\n", Engine::standart->getHandleByHash(torrent_hash)->file_priority(0));
}

extern "C" Files get_files_of_torrent_by_path(char* torrent_path) {
    torrent_info* info = get_torrent(torrent_path);
    return get_files_of_torrent(info);
}

extern "C" Files get_files_of_torrent_by_hash(char* torrent_hash) {
    torrent_handle* handle = Engine::standart->getHandleByHash(torrent_hash);
    if (handle == NULL) {
        Files files {
            .error = 1
        };
        return files;
    }
    torrent_info* info = (torrent_info*)&handle->get_torrent_info();
    Files files = get_files_of_torrent(info);
    for (int i = 0; i < files.size; i++) {
        files.file_priority[i] = handle->file_priority(i);
    }
    return files;
}

extern "C" void save_fast_resume() {
    //int outstanding_resume_data = 0; // global counter of outstanding resume data
    session* ses = Engine::standart->s;
    std::ofstream out((Engine::standart->root_path + "/_Config/state.fastresume").c_str(), std::ios_base::binary);
    out.unsetf(std::ios_base::skipws);
    lt::entry save;
    ses->save_state(save);
    bencode(std::ostream_iterator<char>(out), save);
    
    
    
//    std::vector<torrent_handle> handles = ses->get_torrents();
//    ses->pause();
//    for (std::vector<torrent_handle>::iterator i = handles.begin();
//         i != handles.end(); ++i)
//    {
//        torrent_handle& h = *i;
//        if (!h.is_valid()) continue;
//        torrent_status s = h.status();
//        if (!s.has_metadata) continue;
//        if (!s.need_save_resume) continue;
//
//        h.save_resume_data();
//        ++outstanding_resume_data;
//    }
//
//    while (outstanding_resume_data > 0)
//    {
//        alert const* a = ses->wait_for_alert(seconds(10));
//
//        // if we don't get an alert within 10 seconds, abort
//        if (a == 0) break;
//
//        std::auto_ptr<alert> holder = ses->pop_alert();
//
//        if (alert_cast<save_resume_data_failed_alert>(a))
//        {
//            //process_alert(a);
//            --outstanding_resume_data;
//            continue;
//        }
//
//        save_resume_data_alert const* rd = alert_cast<save_resume_data_alert>(a);
//        if (rd == 0)
//        {
//            //process_alert(a);
//            continue;
//        }
//
//        torrent_handle h = rd->handle;
//        std::ofstream out((h.save_path() + "/" + h.get_torrent_info().name() + ".fastresume").c_str()
//                          , std::ios_base::binary);
//        out.unsetf(std::ios_base::skipws);
//        bencode(std::ostream_iterator<char>(out), *rd->resume_data);
//        --outstanding_resume_data;
//    }
    printf("SAVED!!");
}

extern "C" Result getTorrentInfo() {
    std::string state_str[] = {"Queued", "Hashing", "Metadata", "Downloading", "Finished", "Seeding", "Allocating", "Checking fastresume"};
    
    int size = (int)Engine::standart->handlers.size();
    Result res{
        .count = size,
        .name = new char*[size],
        .state = new char*[size],
        .hash = new char*[size],
        .creator = new char*[size],
        .comment = new char*[size],
        .total_wanted = new long long[size],
        .total_wanted_done = new long long[size],
        .progress = new float[size],
        .download_rate = new int[size],
        .upload_rate = new int[size],
        .total_download = new long long[size],
        .total_upload = new long long[size],
        .num_seeds = new int[size],
        .num_peers = new int[size],
        .total_size = new long long[size],
        .total_done = new long long[size],
        .creation_date = new time_t[size]
    };
    
    for(int i = 0; i < Engine::standart->handlers.size(); i++) {
        torrent_status stat = Engine::standart->handlers[i].status();
        torrent_info* info = NULL;
        if (stat.has_metadata) {
            info = (torrent_info*)&Engine::standart->handlers[i].get_torrent_info();
        }
        
        res.name[i] = new char[stat.name.length() + 1];
        strcpy((char*)res.name[i], stat.name.c_str());
        
        res.state[i] = new char[state_str[stat.state].length() + 1];
        strcpy((char*)res.state[i], state_str[stat.state].c_str());
        
        std::string hash = stat.hash_to_string();
        res.hash[i] = new char[hash.length() + 1];
        strcpy(res.hash[i], hash.c_str());
        
        if (info != NULL) {
            res.creator[i] = new char[info->creator().length() + 1];
            strcpy((char*)res.creator[i], info->creator().c_str());
        } else {
            res.creator[i] = new char[1];
        }
        
        if (info != NULL) {
            res.comment[i] = new char[info->comment().length() + 1];
            strcpy(res.comment[i], info->comment().c_str());
        } else {
            res.comment[i] = new char[1];
        }
        
        res.progress[i] = stat.progress;
        
        res.total_wanted[i] = stat.total_wanted;
        
        res.total_wanted_done[i] = stat.total_wanted_done;
        
        res.total_size[i] = info != NULL ? info->total_size() : 0;
        
        res.total_done[i] = stat.total_done;
        
        res.download_rate[i] = stat.download_rate;
        
        res.upload_rate[i] = stat.upload_rate;
        
        res.total_download[i] = stat.total_download;
        
        res.total_upload[i] = stat.total_upload;
        
        res.num_seeds[i] = stat.num_seeds;
        
        res.num_peers[i] = stat.num_peers;
        
        res.creation_date[i] = info != NULL ? info->creation_date().value() : 0;
    }
    
    return res;
}