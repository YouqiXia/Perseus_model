// See LICENSE for license details.



#include "spikeInterface.hpp"
#include "byteorder.h"

static void help(int exit_code = 1)
{
  fprintf(stderr, "Spike RISC-V ISA Simulator " SPIKE_VERSION "\n\n");
  fprintf(stderr, "usage: spike [host options] <target program> [target options]\n");
  fprintf(stderr, "Host Options:\n");
  fprintf(stderr, "  -p<n>                 Simulate <n> processors [default 1]\n");
  fprintf(stderr, "  -m<n>                 Provide <n> MiB of target memory [default 2048]\n");
  fprintf(stderr, "  -m<a:m,b:n,...>       Provide memory regions of size m and n bytes\n");
  fprintf(stderr, "                          at base addresses a and b (with 4 KiB alignment)\n");
  fprintf(stderr, "  -d                    Interactive debug mode\n");
  fprintf(stderr, "  -g                    Track histogram of PCs\n");
  fprintf(stderr, "  -l                    Generate a log of execution\n");
#ifdef HAVE_BOOST_ASIO
  fprintf(stderr, "  -s                    Command I/O via socket (use with -d)\n");
#endif
  fprintf(stderr, "  -h, --help            Print this help message\n");
  fprintf(stderr, "  -H                    Start halted, allowing a debugger to connect\n");
  fprintf(stderr, "  --log=<name>          File name for option -l\n");
  fprintf(stderr, "  --debug-cmd=<name>    Read commands from file (use with -d)\n");
  fprintf(stderr, "  --isa=<name>          RISC-V ISA string [default %s]\n", DEFAULT_ISA);
  fprintf(stderr, "  --pmpregions=<n>      Number of PMP regions [default 16]\n");
  fprintf(stderr, "  --pmpgranularity=<n>  PMP Granularity in bytes [default 4]\n");
  fprintf(stderr, "  --priv=<m|mu|msu>     RISC-V privilege modes supported [default %s]\n", DEFAULT_PRIV);
  fprintf(stderr, "  --varch=<name>        RISC-V Vector uArch string [default %s]\n", DEFAULT_VARCH);
  fprintf(stderr, "  --pc=<address>        Override ELF entry point\n");
  fprintf(stderr, "  --hartids=<a,b,...>   Explicitly specify hartids, default is 0,1,...\n");
  fprintf(stderr, "  --ic=<S>:<W>:<B>      Instantiate a cache model with S sets,\n");
  fprintf(stderr, "  --dc=<S>:<W>:<B>        W ways, and B-byte blocks (with S and\n");
  fprintf(stderr, "  --l2=<S>:<W>:<B>        B both powers of 2).\n");
  fprintf(stderr, "  --big-endian          Use a big-endian memory system.\n");
  fprintf(stderr, "  --misaligned          Support misaligned memory accesses\n");
  fprintf(stderr, "  --device=<name>       Attach MMIO plugin device from an --extlib library\n");
  fprintf(stderr, "  --log-cache-miss      Generate a log of cache miss\n");
  fprintf(stderr, "  --log-commits         Generate a log of commits info\n");
  fprintf(stderr, "  --extension=<name>    Specify RoCC Extension\n");
  fprintf(stderr, "                          This flag can be used multiple times.\n");
  fprintf(stderr, "  --extlib=<name>       Shared library to load\n");
  fprintf(stderr, "                        This flag can be used multiple times.\n");
  fprintf(stderr, "  --rbb-port=<port>     Listen on <port> for remote bitbang connection\n");
  fprintf(stderr, "  --dump-dts            Print device tree string and exit\n");
  fprintf(stderr, "  --dtb=<path>          Use specified device tree blob [default: auto-generate]\n");
  fprintf(stderr, "  --disable-dtb         Don't write the device tree blob into memory\n");
  fprintf(stderr, "  --kernel=<path>       Load kernel flat image into memory\n");
  fprintf(stderr, "  --initrd=<path>       Load kernel initrd into memory\n");
  fprintf(stderr, "  --bootargs=<args>     Provide custom bootargs for kernel [default: %s]\n",
          DEFAULT_KERNEL_BOOTARGS);
  fprintf(stderr, "  --real-time-clint     Increment clint time at real-time rate\n");
  fprintf(stderr, "  --triggers=<n>        Number of supported triggers [default 4]\n");
  fprintf(stderr, "  --dm-progsize=<words> Progsize for the debug module [default 2]\n");
  fprintf(stderr, "  --dm-sba=<bits>       Debug system bus access supports up to "
      "<bits> wide accesses [default 0]\n");
  fprintf(stderr, "  --dm-auth             Debug module requires debugger to authenticate\n");
  fprintf(stderr, "  --dmi-rti=<n>         Number of Run-Test/Idle cycles "
      "required for a DMI access [default 0]\n");
  fprintf(stderr, "  --dm-abstract-rti=<n> Number of Run-Test/Idle cycles "
      "required for an abstract command to execute [default 0]\n");
  fprintf(stderr, "  --dm-no-hasel         Debug module supports hasel\n");
  fprintf(stderr, "  --dm-no-abstract-csr  Debug module won't support abstract CSR access\n");
  fprintf(stderr, "  --dm-no-abstract-fpr  Debug module won't support abstract FPR access\n");
  fprintf(stderr, "  --dm-no-halt-groups   Debug module won't support halt groups\n");
  fprintf(stderr, "  --dm-no-impebreak     Debug module won't support implicit ebreak in program buffer\n");
  fprintf(stderr, "  --blocksz=<size>      Cache block size (B) for CMO operations(powers of 2) [default 64]\n");

  exit(exit_code);
}

static void suggest_help()
{
  fprintf(stderr, "Try 'spike --help' for more information.\n");
  exit(1);
}

static bool check_file_exists(const char *fileName)
{
  std::ifstream infile(fileName);
  return infile.good();
}

static std::ifstream::pos_type get_file_size(const char *filename)
{
  std::ifstream in(filename, std::ios::ate | std::ios::binary);
  return in.tellg();
}

static void read_file_bytes(const char *filename,size_t fileoff,
                            abstract_mem_t* mem, size_t memoff, size_t read_sz)
{
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  in.seekg(fileoff, std::ios::beg);

  std::vector<char> read_buf(read_sz, 0);
  in.read(&read_buf[0], read_sz);
  mem->store(memoff, read_sz, (uint8_t*)&read_buf[0]);
}

bool sort_mem_region(const mem_cfg_t &a, const mem_cfg_t &b)
{
  if (a.get_base() == b.get_base())
    return (a.get_size() < b.get_size());
  else
    return (a.get_base() < b.get_base());
}

static bool check_mem_overlap(const mem_cfg_t& L, const mem_cfg_t& R)
{
  return std::max(L.get_base(), R.get_base()) <= std::min(L.get_inclusive_end(), R.get_inclusive_end());
}

static bool check_if_merge_covers_64bit_space(const mem_cfg_t& L,
                                              const mem_cfg_t& R)
{
  if (!check_mem_overlap(L, R))
    return false;

  auto start = std::min(L.get_base(), R.get_base());
  auto end = std::max(L.get_inclusive_end(), R.get_inclusive_end());

  return (start == 0ull) && (end == std::numeric_limits<uint64_t>::max());
}

static mem_cfg_t merge_mem_regions(const mem_cfg_t& L, const mem_cfg_t& R)
{
  // one can merge only intersecting regions
  assert(check_mem_overlap(L, R));

  const auto merged_base = std::min(L.get_base(), R.get_base());
  const auto merged_end_incl = std::max(L.get_inclusive_end(), R.get_inclusive_end());
  const auto merged_size = merged_end_incl - merged_base + 1;

  return mem_cfg_t(merged_base, merged_size);
}

// check the user specified memory regions and merge the overlapping or
// eliminate the containing parts
static std::vector<mem_cfg_t>
merge_overlapping_memory_regions(std::vector<mem_cfg_t> mems)
{
  if (mems.empty())
    return {};

  std::sort(mems.begin(), mems.end(), sort_mem_region);

  std::vector<mem_cfg_t> merged_mem;
  merged_mem.push_back(mems.front());

  for (auto mem_it = std::next(mems.begin()); mem_it != mems.end(); ++mem_it) {
    const auto& mem_int = *mem_it;
    if (!check_mem_overlap(merged_mem.back(), mem_int)) {
      merged_mem.push_back(mem_int);
      continue;
    }
    // there is a weird corner case preventing two memory regions from being
    // merged: if the resulting size of a region is 2^64 bytes - currently,
    // such regions are not representable by mem_cfg_t class (because the
    // actual size field is effectively a 64 bit value)
    // so we create two smaller memory regions that total for 2^64 bytes as
    // a workaround
    if (check_if_merge_covers_64bit_space(merged_mem.back(), mem_int)) {
      merged_mem.clear();
      merged_mem.push_back(mem_cfg_t(0ull, 0ull - PGSIZE));
      merged_mem.push_back(mem_cfg_t(0ull - PGSIZE, PGSIZE));
      break;
    }
    merged_mem.back() = merge_mem_regions(merged_mem.back(), mem_int);
  }

  return merged_mem;
}

static std::vector<mem_cfg_t> parse_mem_layout(const char* arg)
{
  std::vector<mem_cfg_t> res;

  // handle legacy mem argument
  char* p;
  auto mb = strtoull(arg, &p, 0);
  if (*p == 0) {
    reg_t size = reg_t(mb) << 20;
    if (size != (size_t)size)
      throw std::runtime_error("Size would overflow size_t");
    res.push_back(mem_cfg_t(reg_t(DRAM_BASE), size));
    return res;
  }

  // handle base/size tuples
  while (true) {
    auto base = strtoull(arg, &p, 0);
    if (!*p || *p != ':')
      help();
    auto size = strtoull(p + 1, &p, 0);

    // page-align base and size
    auto base0 = base, size0 = size;
    size += base0 % PGSIZE;
    base -= base0 % PGSIZE;
    if (size % PGSIZE != 0)
      size += PGSIZE - size % PGSIZE;

    if (size != size0) {
      fprintf(stderr, "Warning: the memory at [0x%llX, 0x%llX] has been realigned\n"
                      "to the %ld KiB page size: [0x%llX, 0x%llX]\n",
              base0, base0 + size0 - 1, long(PGSIZE / 1024), base, base + size - 1);
    }

    if (!mem_cfg_t::check_if_supported(base, size)) {
      fprintf(stderr, "Unsupported memory region "
                      "{base = 0x%llX, size = 0x%llX} specified\n",
              base, size);
      exit(EXIT_FAILURE);
    }

    const unsigned long long max_allowed_pa = (1ull << MAX_PADDR_BITS) - 1ull;
    assert(max_allowed_pa <= std::numeric_limits<reg_t>::max());
    mem_cfg_t mem_region(base, size);
    if (mem_region.get_inclusive_end() > max_allowed_pa) {
      int bits_required = 64 - clz(mem_region.get_inclusive_end());
      fprintf(stderr, "Unsupported memory region "
                      "{base = 0x%" PRIX64 ", size = 0x%" PRIX64 "} specified,"
                      " which requires %d bits of physical address\n"
                      "    The largest accessible physical address "
                      "is 0x%llX (defined by MAX_PADDR_BITS constant, which is %d)\n",
              mem_region.get_base(), mem_region.get_size(), bits_required,
              max_allowed_pa, MAX_PADDR_BITS);
      exit(EXIT_FAILURE);
    }

    res.push_back(mem_region);

    if (!*p)
      break;
    if (*p != ',')
      help();
    arg = p + 1;
  }

  auto merged_mem = merge_overlapping_memory_regions(res);

  assert(!merged_mem.empty());
  return merged_mem;
}

static std::vector<std::pair<reg_t, abstract_mem_t*>> make_mems(const std::vector<mem_cfg_t> &layout)
{
  std::vector<std::pair<reg_t, abstract_mem_t*>> mems;
  mems.reserve(layout.size());
  for (const auto &cfg : layout) {
    mems.push_back(std::make_pair(cfg.get_base(), new mem_t(cfg.get_size())));
  }
  return mems;
}

static unsigned long atoul_safe(const char* s)
{
  char* e;
  auto res = strtoul(s, &e, 10);
  if (*e)
    help();
  return res;
}

static unsigned long atoul_nonzero_safe(const char* s)
{
  auto res = atoul_safe(s);
  if (!res)
    help();
  return res;
}

static std::vector<size_t> parse_hartids(const char *s)
{
  std::string const str(s);
  std::stringstream stream(str);
  std::vector<size_t> hartids;

  int n;
  while (stream >> n) {
    if (n < 0) {
      fprintf(stderr, "Negative hart ID %d is unsupported\n", n);
      exit(-1);
    }

    hartids.push_back(n);
    if (stream.peek() == ',') stream.ignore();
  }

  if (hartids.empty()) {
    fprintf(stderr, "No hart IDs specified\n");
    exit(-1);
  }

  std::sort(hartids.begin(), hartids.end());

  const auto dup = std::adjacent_find(hartids.begin(), hartids.end());
  if (dup != hartids.end()) {
    fprintf(stderr, "Duplicate hart ID %zu\n", *dup);
    exit(-1);
  }

  return hartids;
}

int spikeAdapter::spikeInit(std::vector<std::string>& commandLineArgs){
    int argc = static_cast<int>(commandLineArgs.size());
    char** argv = new char*[argc];
    for (int i = 0; i < argc; ++i) {
        argv[i] = strdup(commandLineArgs[i].c_str());
    }

    return spikeInit_(argc, argv);
}

int spikeAdapter::spikeFastInit(std::vector<std::string>& commandLineArgs){
    int argc = static_cast<int>(commandLineArgs.size());
    char** argv = new char*[argc];
    for (int i = 0; i < argc; ++i) {
        argv[i] = strdup(commandLineArgs[i].c_str());
    }

    return spikeFastInit_(argc, argv);
}

int spikeAdapter::spikeInit_(int argc, char** argv)
{
    bool debug = false;
    bool halted = false;
    bool histogram = false;
    bool log = false;
    bool UNUSED socket = false;  // command line option -s
    bool dump_dts = false;
    bool dtb_enabled = true;
    const char* kernel = NULL;
    reg_t kernel_offset, kernel_size;
    std::vector<device_factory_t*> plugin_device_factories;
    std::unique_ptr<icache_sim_t> ic;
    std::unique_ptr<dcache_sim_t> dc;
    std::unique_ptr<cache_sim_t> l2;
    bool log_cache = false;
    bool log_commits = false;
    const char *log_path = nullptr;
    std::vector<std::function<extension_t*()>> extensions;
    const char* initrd = NULL;
    const char* dtb_file = NULL;
    uint16_t rbb_port = 0;
    bool use_rbb = false;
    unsigned dmi_rti = 0;
    reg_t blocksz = 64;
    debug_module_config_t dm_config;
    cfg_arg_t<size_t> nprocs(1);


    auto const device_parser = [&plugin_device_factories](const char *s) {
        const std::string device_args(s);
        std::vector<std::string> parsed_args;
        std::stringstream sstr(device_args);
        while (sstr.good()) {
            std::string substr;
            getline(sstr, substr, ',');
            parsed_args.push_back(substr);
        }
        if (parsed_args.empty()) throw std::runtime_error("Plugin argument is empty.");

        const std::string name = parsed_args[0];
        if (name.empty()) throw std::runtime_error("Plugin name is empty.");

        auto it = mmio_device_map().find(name);
        if (it == mmio_device_map().end()) throw std::runtime_error("Plugin \"" + name + "\" not found in loaded extlibs.");

        parsed_args.erase(parsed_args.begin());
        it->second->set_sargs(parsed_args);
        plugin_device_factories.push_back(it->second);
    };

    option_parser_t parser;
    parser.help(&suggest_help);
    parser.option('h', "help", 0, [&](const char UNUSED *s){help(0);});
    parser.option('d', 0, 0, [&](const char UNUSED *s){debug = true;});
    parser.option('g', 0, 0, [&](const char UNUSED *s){histogram = true;});
    parser.option('l', 0, 0, [&](const char UNUSED *s){log = true;});
#ifdef HAVE_BOOST_ASIO
    parser.option('s', 0, 0, [&](const char UNUSED *s){socket = true;});
#endif
    parser.option('p', 0, 1, [&](const char* s){nprocs = atoul_nonzero_safe(s);});
    parser.option('m', 0, 1, [&](const char* s){cfg.mem_layout = parse_mem_layout(s);});
    // I wanted to use --halted, but for some reason that doesn't work.
    parser.option('H', 0, 0, [&](const char UNUSED *s){halted = true;});
    parser.option(0, "rbb-port", 1, [&](const char* s){use_rbb = true; rbb_port = atoul_safe(s);});
    parser.option(0, "pc", 1, [&](const char* s){cfg.start_pc = strtoull(s, 0, 0);});
    parser.option(0, "hartids", 1, [&](const char* s){
        cfg.hartids = parse_hartids(s);
        cfg.explicit_hartids = true;
    });
    parser.option(0, "ic", 1, [&](const char* s){ic.reset(new icache_sim_t(s));});
    parser.option(0, "dc", 1, [&](const char* s){dc.reset(new dcache_sim_t(s));});
    parser.option(0, "l2", 1, [&](const char* s){l2.reset(cache_sim_t::construct(s, "L2$"));});
    parser.option(0, "big-endian", 0, [&](const char UNUSED *s){cfg.endianness = endianness_big;});
    parser.option(0, "misaligned", 0, [&](const char UNUSED *s){cfg.misaligned = true;});
    parser.option(0, "log-cache-miss", 0, [&](const char UNUSED *s){log_cache = true;});
    parser.option(0, "isa", 1, [&](const char* s){cfg.isa = s;});
    parser.option(0, "pmpregions", 1, [&](const char* s){cfg.pmpregions = atoul_safe(s);});
    parser.option(0, "pmpgranularity", 1, [&](const char* s){cfg.pmpgranularity = atoul_safe(s);});
    parser.option(0, "priv", 1, [&](const char* s){cfg.priv = s;});
    parser.option(0, "varch", 1, [&](const char* s){cfg.varch = s;});
    parser.option(0, "device", 1, device_parser);
    parser.option(0, "extension", 1, [&](const char* s){extensions.push_back(find_extension(s));});
    parser.option(0, "dump-dts", 0, [&](const char UNUSED *s){dump_dts = true;});
    parser.option(0, "disable-dtb", 0, [&](const char UNUSED *s){dtb_enabled = false;});
    parser.option(0, "dtb", 1, [&](const char *s){dtb_file = s;});
    parser.option(0, "kernel", 1, [&](const char* s){kernel = s;});
    parser.option(0, "initrd", 1, [&](const char* s){initrd = s;});
    parser.option(0, "bootargs", 1, [&](const char* s){cfg.bootargs = s;});
    parser.option(0, "real-time-clint", 0, [&](const char UNUSED *s){cfg.real_time_clint = true;});
    parser.option(0, "triggers", 1, [&](const char *s){cfg.trigger_count = atoul_safe(s);});
    parser.option(0, "extlib", 1, [&](const char *s){
        void *lib = dlopen(s, RTLD_NOW | RTLD_GLOBAL);
        if (lib == NULL) {
            fprintf(stderr, "Unable to load extlib '%s': %s\n", s, dlerror());
            exit(-1);
        }
    });
    parser.option(0, "dm-progsize", 1,
                  [&](const char* s){dm_config.progbufsize = atoul_safe(s);});
    parser.option(0, "dm-no-impebreak", 0,
                  [&](const char UNUSED *s){dm_config.support_impebreak = false;});
    parser.option(0, "dm-sba", 1,
                  [&](const char* s){dm_config.max_sba_data_width = atoul_safe(s);});
    parser.option(0, "dm-auth", 0,
                  [&](const char UNUSED *s){dm_config.require_authentication = true;});
    parser.option(0, "dmi-rti", 1,
                  [&](const char* s){dmi_rti = atoul_safe(s);});
    parser.option(0, "dm-abstract-rti", 1,
                  [&](const char* s){dm_config.abstract_rti = atoul_safe(s);});
    parser.option(0, "dm-no-hasel", 0,
                  [&](const char UNUSED *s){dm_config.support_hasel = false;});
    parser.option(0, "dm-no-abstract-csr", 0,
                  [&](const char UNUSED *s){dm_config.support_abstract_csr_access = false;});
    parser.option(0, "dm-no-abstract-fpr", 0,
                  [&](const char UNUSED *s){dm_config.support_abstract_fpr_access = false;});
    parser.option(0, "dm-no-halt-groups", 0,
                  [&](const char UNUSED *s){dm_config.support_haltgroups = false;});
    parser.option(0, "log-commits", 0,
                  [&](const char UNUSED *s){log_commits = true;});
    parser.option(0, "log", 1,
                  [&](const char* s){log_path = s;});
    FILE *cmd_file = NULL;
    parser.option(0, "debug-cmd", 1, [&](const char* s){
        if ((cmd_file = fopen(s, "r"))==NULL) {
            fprintf(stderr, "Unable to open command file '%s'\n", s);
            exit(-1);
        }
    });
    parser.option(0, "blocksz", 1, [&](const char* s){
        blocksz = strtoull(s, 0, 0);
        const unsigned min_blocksz = 16;
        const unsigned max_blocksz = PGSIZE;
        if (blocksz < min_blocksz || blocksz > max_blocksz || ((blocksz & (blocksz - 1))) != 0) {
            fprintf(stderr, "--blocksz must be a power of 2 between %u and %u\n",
                    min_blocksz, max_blocksz);
            exit(-1);
        }
    });

    auto argv1 = parser.parse(argv);
    std::vector<std::string> htif_args(argv1, (const char*const*)argv + argc);

    if (!*argv1)
        help();

    std::vector<std::pair<reg_t, abstract_mem_t*>> mems =
            make_mems(cfg.mem_layout);

    if (kernel && check_file_exists(kernel)) {
        const char *isa = cfg.isa;
        kernel_size = get_file_size(kernel);
        if (isa[2] == '6' && isa[3] == '4')
            kernel_offset = 0x200000;
        else
            kernel_offset = 0x400000;
        for (auto& m : mems) {
            if (kernel_size && (kernel_offset + kernel_size) < m.second->size()) {
                read_file_bytes(kernel, 0, m.second, kernel_offset, kernel_size);
                break;
            }
        }
    }

    if (initrd && check_file_exists(initrd)) {
        size_t initrd_size = get_file_size(initrd);
        for (auto& m : mems) {
            if (initrd_size && (initrd_size + 0x1000) < m.second->size()) {
                reg_t initrd_end = m.first + m.second->size() - 0x1000;
                reg_t initrd_start = initrd_end - initrd_size;
                cfg.initrd_bounds = std::make_pair(initrd_start, initrd_end);
                read_file_bytes(initrd, 0, m.second, initrd_start - m.first, initrd_size);
                break;
            }
        }
    }

    if (cfg.explicit_hartids) {
        if (nprocs.overridden() && (nprocs() != cfg.nprocs())) {
            std::cerr << "Number of specified hartids ("
                      << cfg.nprocs()
                      << ") doesn't match specified number of processors ("
                      << nprocs() << ").\n";
            exit(1);
        }
    } else {
        // Set default set of hartids based on nprocs, but don't set the
        // explicit_hartids flag (which means that downstream code can know that
        // we've only set the number of harts, not explicitly chosen their IDs).
        std::vector<size_t> default_hartids;
        default_hartids.reserve(nprocs());
        for (size_t i = 0; i < nprocs(); ++i) {
            default_hartids.push_back(i);
        }
        cfg.hartids = default_hartids;
    }

    spike_sim = new sim_t(&cfg, halted,
                          mems, plugin_device_factories, htif_args, dm_config, log_path, dtb_enabled, dtb_file,
                          socket,cmd_file);
    // std::unique_ptr<remote_bitbang_t> remote_bitbang((remote_bitbang_t *) NULL);
    // std::unique_ptr<jtag_dtm_t> jtag_dtm(
    //     new jtag_dtm_t(&s.debug_module, dmi_rti));
    // if (use_rbb) {
    //   remote_bitbang.reset(new remote_bitbang_t(rbb_port, &(*jtag_dtm)));
    //   s.set_remote_bitbang(&(*remote_bitbang));
    // }

    if (dump_dts) {
        printf("%s", spike_sim->get_dts());
        return 0;
    }

    if (ic && l2) ic->set_miss_handler(&*l2);
    if (dc && l2) dc->set_miss_handler(&*l2);
    if (ic) ic->set_log(log_cache);
    if (dc) dc->set_log(log_cache);
    for (size_t i = 0; i < cfg.nprocs(); i++)
    {
        if (ic) spike_sim->get_core(i)->get_mmu()->register_memtracer(&*ic);
        if (dc) spike_sim->get_core(i)->get_mmu()->register_memtracer(&*dc);
        for (auto e : extensions)
            spike_sim->get_core(i)->register_extension(e());
        spike_sim->get_core(i)->get_mmu()->set_cache_blocksz(blocksz);
    }

    spike_sim->set_debug(debug);
    spike_sim->configure_log(log, log_commits);
    spike_sim->set_histogram(histogram);
    return 0;
}

int spikeAdapter::spikeFastInit_(int argc, char** argv)
{
    bool debug = false;
    bool halted = false;
    bool histogram = false;
    bool log = false;
    bool UNUSED socket = false;  // command line option -s
    bool dump_dts = false;
    bool dtb_enabled = true;
    const char* kernel = NULL;
    reg_t kernel_offset, kernel_size;
    std::vector<device_factory_t*> plugin_device_factories;
    std::unique_ptr<icache_sim_t> ic;
    std::unique_ptr<dcache_sim_t> dc;
    std::unique_ptr<cache_sim_t> l2;
    bool log_cache = false;
    bool log_commits = false;
    const char *log_path = nullptr;
    std::vector<std::function<extension_t*()>> extensions;
    const char* initrd = NULL;
    const char* dtb_file = NULL;
    uint16_t rbb_port = 0;
    bool use_rbb = false;
    unsigned dmi_rti = 0;
    reg_t blocksz = 64;
    debug_module_config_t dm_config;
    cfg_arg_t<size_t> nprocs(1);


    auto const device_parser = [&plugin_device_factories](const char *s) {
        const std::string device_args(s);
        std::vector<std::string> parsed_args;
        std::stringstream sstr(device_args);
        while (sstr.good()) {
            std::string substr;
            getline(sstr, substr, ',');
            parsed_args.push_back(substr);
        }
        if (parsed_args.empty()) throw std::runtime_error("Plugin argument is empty.");

        const std::string name = parsed_args[0];
        if (name.empty()) throw std::runtime_error("Plugin name is empty.");

        auto it = mmio_device_map().find(name);
        if (it == mmio_device_map().end()) throw std::runtime_error("Plugin \"" + name + "\" not found in loaded extlibs.");

        parsed_args.erase(parsed_args.begin());
        it->second->set_sargs(parsed_args);
        plugin_device_factories.push_back(it->second);
    };

    option_parser_t parser;
    parser.help(&suggest_help);
    parser.option('h', "help", 0, [&](const char UNUSED *s){help(0);});
    parser.option('d', 0, 0, [&](const char UNUSED *s){debug = true;});
    parser.option('g', 0, 0, [&](const char UNUSED *s){histogram = true;});
    parser.option('l', 0, 0, [&](const char UNUSED *s){log = true;});
#ifdef HAVE_BOOST_ASIO
    parser.option('s', 0, 0, [&](const char UNUSED *s){socket = true;});
#endif
    parser.option('p', 0, 1, [&](const char* s){nprocs = atoul_nonzero_safe(s);});
    parser.option('m', 0, 1, [&](const char* s){cfg_fast.mem_layout = parse_mem_layout(s);});
    // I wanted to use --halted, but for some reason that doesn't work.
    parser.option('H', 0, 0, [&](const char UNUSED *s){halted = true;});
    parser.option(0, "rbb-port", 1, [&](const char* s){use_rbb = true; rbb_port = atoul_safe(s);});
    parser.option(0, "pc", 1, [&](const char* s){cfg_fast.start_pc = strtoull(s, 0, 0);});
    parser.option(0, "hartids", 1, [&](const char* s){
        cfg_fast.hartids = parse_hartids(s);
        cfg_fast.explicit_hartids = true;
    });
    parser.option(0, "ic", 1, [&](const char* s){ic.reset(new icache_sim_t(s));});
    parser.option(0, "dc", 1, [&](const char* s){dc.reset(new dcache_sim_t(s));});
    parser.option(0, "l2", 1, [&](const char* s){l2.reset(cache_sim_t::construct(s, "L2$"));});
    parser.option(0, "big-endian", 0, [&](const char UNUSED *s){cfg_fast.endianness = endianness_big;});
    parser.option(0, "misaligned", 0, [&](const char UNUSED *s){cfg_fast.misaligned = true;});
    parser.option(0, "log-cache-miss", 0, [&](const char UNUSED *s){log_cache = true;});
    parser.option(0, "isa", 1, [&](const char* s){cfg_fast.isa = s;});
    parser.option(0, "pmpregions", 1, [&](const char* s){cfg_fast.pmpregions = atoul_safe(s);});
    parser.option(0, "pmpgranularity", 1, [&](const char* s){cfg_fast.pmpgranularity = atoul_safe(s);});
    parser.option(0, "priv", 1, [&](const char* s){cfg_fast.priv = s;});
    parser.option(0, "varch", 1, [&](const char* s){cfg_fast.varch = s;});
    parser.option(0, "device", 1, device_parser);
    parser.option(0, "extension", 1, [&](const char* s){extensions.push_back(find_extension(s));});
    parser.option(0, "dump-dts", 0, [&](const char UNUSED *s){dump_dts = true;});
    parser.option(0, "disable-dtb", 0, [&](const char UNUSED *s){dtb_enabled = false;});
    parser.option(0, "dtb", 1, [&](const char *s){dtb_file = s;});
    parser.option(0, "kernel", 1, [&](const char* s){kernel = s;});
    parser.option(0, "initrd", 1, [&](const char* s){initrd = s;});
    parser.option(0, "bootargs", 1, [&](const char* s){cfg_fast.bootargs = s;});
    parser.option(0, "real-time-clint", 0, [&](const char UNUSED *s){cfg_fast.real_time_clint = true;});
    parser.option(0, "triggers", 1, [&](const char *s){cfg_fast.trigger_count = atoul_safe(s);});
    parser.option(0, "extlib", 1, [&](const char *s){
        void *lib = dlopen(s, RTLD_NOW | RTLD_GLOBAL);
        if (lib == NULL) {
            fprintf(stderr, "Unable to load extlib '%s': %s\n", s, dlerror());
            exit(-1);
        }
    });
    parser.option(0, "dm-progsize", 1,
                  [&](const char* s){dm_config.progbufsize = atoul_safe(s);});
    parser.option(0, "dm-no-impebreak", 0,
                  [&](const char UNUSED *s){dm_config.support_impebreak = false;});
    parser.option(0, "dm-sba", 1,
                  [&](const char* s){dm_config.max_sba_data_width = atoul_safe(s);});
    parser.option(0, "dm-auth", 0,
                  [&](const char UNUSED *s){dm_config.require_authentication = true;});
    parser.option(0, "dmi-rti", 1,
                  [&](const char* s){dmi_rti = atoul_safe(s);});
    parser.option(0, "dm-abstract-rti", 1,
                  [&](const char* s){dm_config.abstract_rti = atoul_safe(s);});
    parser.option(0, "dm-no-hasel", 0,
                  [&](const char UNUSED *s){dm_config.support_hasel = false;});
    parser.option(0, "dm-no-abstract-csr", 0,
                  [&](const char UNUSED *s){dm_config.support_abstract_csr_access = false;});
    parser.option(0, "dm-no-abstract-fpr", 0,
                  [&](const char UNUSED *s){dm_config.support_abstract_fpr_access = false;});
    parser.option(0, "dm-no-halt-groups", 0,
                  [&](const char UNUSED *s){dm_config.support_haltgroups = false;});
    parser.option(0, "log-commits", 0,
                  [&](const char UNUSED *s){log_commits = true;});
    parser.option(0, "log", 1,
                  [&](const char* s){log_path = s;});
    FILE *cmd_file = NULL;
    parser.option(0, "debug-cmd", 1, [&](const char* s){
        if ((cmd_file = fopen(s, "r"))==NULL) {
            fprintf(stderr, "Unable to open command file '%s'\n", s);
            exit(-1);
        }
    });
    parser.option(0, "blocksz", 1, [&](const char* s){
        blocksz = strtoull(s, 0, 0);
        const unsigned min_blocksz = 16;
        const unsigned max_blocksz = PGSIZE;
        if (blocksz < min_blocksz || blocksz > max_blocksz || ((blocksz & (blocksz - 1))) != 0) {
            fprintf(stderr, "--blocksz must be a power of 2 between %u and %u\n",
                    min_blocksz, max_blocksz);
            exit(-1);
        }
    });

    auto argv1 = parser.parse(argv);
    std::vector<std::string> htif_args(argv1, (const char*const*)argv + argc);

    if (!*argv1)
        help();

    std::vector<std::pair<reg_t, abstract_mem_t*>> mems =
            make_mems(cfg_fast.mem_layout);

    if (kernel && check_file_exists(kernel)) {
        const char *isa = cfg_fast.isa;
        kernel_size = get_file_size(kernel);
        if (isa[2] == '6' && isa[3] == '4')
            kernel_offset = 0x200000;
        else
            kernel_offset = 0x400000;
        for (auto& m : mems) {
            if (kernel_size && (kernel_offset + kernel_size) < m.second->size()) {
                read_file_bytes(kernel, 0, m.second, kernel_offset, kernel_size);
                break;
            }
        }
    }

    if (initrd && check_file_exists(initrd)) {
        size_t initrd_size = get_file_size(initrd);
        for (auto& m : mems) {
            if (initrd_size && (initrd_size + 0x1000) < m.second->size()) {
                reg_t initrd_end = m.first + m.second->size() - 0x1000;
                reg_t initrd_start = initrd_end - initrd_size;
                cfg_fast.initrd_bounds = std::make_pair(initrd_start, initrd_end);
                read_file_bytes(initrd, 0, m.second, initrd_start - m.first, initrd_size);
                break;
            }
        }
    }

    if (cfg_fast.explicit_hartids) {
        if (nprocs.overridden() && (nprocs() != cfg_fast.nprocs())) {
            std::cerr << "Number of specified hartids ("
                      << cfg_fast.nprocs()
                      << ") doesn't match specified number of processors ("
                      << nprocs() << ").\n";
            exit(1);
        }
    } else {
        // Set default set of hartids based on nprocs, but don't set the
        // explicit_hartids flag (which means that downstream code can know that
        // we've only set the number of harts, not explicitly chosen their IDs).
        std::vector<size_t> default_hartids;
        default_hartids.reserve(nprocs());
        for (size_t i = 0; i < nprocs(); ++i) {
            default_hartids.push_back(i);
        }
        cfg_fast.hartids = default_hartids;
    }

    spike_sim_fast = new sim_t(&cfg_fast, halted,
                          mems, plugin_device_factories, htif_args, dm_config, log_path, dtb_enabled, dtb_file,
                          socket,cmd_file);
    // std::unique_ptr<remote_bitbang_t> remote_bitbang((remote_bitbang_t *) NULL);
    // std::unique_ptr<jtag_dtm_t> jtag_dtm(
    //     new jtag_dtm_t(&s.debug_module, dmi_rti));
    // if (use_rbb) {
    //   remote_bitbang.reset(new remote_bitbang_t(rbb_port, &(*jtag_dtm)));
    //   s.set_remote_bitbang(&(*remote_bitbang));
    // }

    if (dump_dts) {
        printf("%s", spike_sim_fast->get_dts());
        return 0;
    }

    if (ic && l2) ic->set_miss_handler(&*l2);
    if (dc && l2) dc->set_miss_handler(&*l2);
    if (ic) ic->set_log(log_cache);
    if (dc) dc->set_log(log_cache);
    for (size_t i = 0; i < cfg_fast.nprocs(); i++)
    {
        if (ic) spike_sim_fast->get_core(i)->get_mmu()->register_memtracer(&*ic);
        if (dc) spike_sim_fast->get_core(i)->get_mmu()->register_memtracer(&*dc);
        for (auto e : extensions)
            spike_sim_fast->get_core(i)->register_extension(e());
        spike_sim_fast->get_core(i)->get_mmu()->set_cache_blocksz(blocksz);
    }

    spike_sim_fast->set_debug(debug);
    spike_sim_fast->configure_log(log, log_commits);
    spike_sim_fast->set_histogram(histogram);
    return 0;
}

spikeAdapter* spikeAdapter::spike_adapter_ = nullptr;

spikeAdapter* spikeAdapter::getSpikeAdapter() {
    if (spike_adapter_ == nullptr) {
        spike_adapter_ = new spikeAdapter();
    }
    return spike_adapter_;
}

//get from g_spike_tunnel
spikeInsnPtr spikeAdapter::spikeGetNextInst(){
    spikeInsnPtr insn = spike_tunnel;
    spike_tunnel = nullptr;
    return insn;
}

void spikeAdapter::decodeHook(void * in, uint64_t pc, uint64_t npc){
    //npc != PC_SERIALIZE_BEFORE
    if (unlikely(npc == 3)){
      return;
    }

    if (unlikely(npc == 5)) {
        spike_npc_ = spike_sim->procs[0]->get_state()->pc;
    }
    // std::cout << "spike pc: 0x" << std::hex << pc << " next pc: 0x" << npc << std::dec <<std::endl;
    spike_tunnel = std::make_shared<spike_insn>((insn_fetch_t*) in, pc);
}

bool spikeAdapter::commitHook(){
    if(spike_tunnel == nullptr) {
        return true;
    }

    //same cycle same insn with tail
    processor_t *p = spike_sim->procs[0];

    auto& reg = p->get_state()->log_reg_write;
    auto& load = p->get_state()->log_mem_read;
    auto& store = p->get_state()->log_mem_write;
    int priv = p->get_state()->last_inst_priv;
    int xlen = p->get_state()->last_inst_xlen;
    int flen = p->get_state()->last_inst_flen;
    bool show_vec = false;

    spike_tunnel->spike_log_reg_write.clear();
    spike_tunnel->spike_log_reg_write = spike_sim->procs[0]->get_state()->log_reg_write;
    spike_tunnel->reg_write.clear();
    // parse log_reg_write
    for (auto item : spike_tunnel->spike_log_reg_write)
    {
        if (item.first == 0)
            continue;

        char prefix = ' ';
        int size;
        int rd = item.first >> 4;
        bool is_vec = false;
        bool is_vreg = false;
        switch (item.first & 0xf)
        {
            case 0:
                size = xlen;
                prefix = 'x';
                break;
            case 1:
                size = flen;
                prefix = 'f';
                break;
            case 2:
                size = p->VU.VLEN;
                prefix = 'v';
                is_vreg = true;
                break;
            case 3:
                is_vec = true;
                break;
            case 4:
                size = xlen;
                prefix = 'c';
                break;
            default:
                assert("can't been here" && 0);
                break;
        }

        // if (!show_vec && (is_vreg || is_vec))
        // {
        //     fprintf(log_file, " e%ld %s%ld l%ld",
        //             (long)p->VU.vsew,
        //             p->VU.vflmul < 1 ? "mf" : "m",
        //             p->VU.vflmul < 1 ? (long)(1 / p->VU.vflmul) : (long)p->VU.vflmul,
        //             (long)p->VU.vl->read());
        //     show_vec = true;
        // }

        if (!is_vec)
        {
            std::string reg_name;

            // malloc mem for reg value
            size_t bytes = size / 8;
            size_t *v = (size_t *)malloc(bytes);

            if (prefix == 'c')
                reg_name = csr_name(rd);
            else
                reg_name = std::string(1, prefix) + std::to_string(rd);

            spike_tunnel->reg_write.push_back(std::make_tuple(reg_name, bytes, v));

            if (is_vreg)
                memcpy(v, &p->VU.elt<uint8_t>(rd, 0), bytes);
            else
                memcpy(v, &item.second.v, bytes);
        }
    }

    spike_tunnel->spike_log_mem_read.clear();
    spike_tunnel->spike_log_mem_read = spike_sim->procs[0]->get_state()->log_mem_read;
    spike_tunnel->mem_read.clear();
    // parse log_mem_read
    for (auto item : spike_tunnel->spike_log_mem_read) {
        size_t bytes = std::get<2>(item) << 3;

        spike_tunnel->mem_read.push_back(std::make_tuple((std::get<0>(item)), bytes));
    }

    spike_tunnel->spike_log_mem_write.clear();
    spike_tunnel->spike_log_mem_write = spike_sim->procs[0]->get_state()->log_mem_write;
    spike_tunnel->mem_write.clear();
    for (auto item : spike_tunnel->spike_log_mem_write) {
        size_t bytes = std::get<2>(item) << 3;
        size_t *v = (size_t *)malloc(bytes);
        memcpy(v, &std::get<1>(item), bytes);

        spike_tunnel->mem_write.push_back(std::make_tuple((std::get<0>(item)), bytes, v));
    }

    spike_tunnel->spike_last_inst_priv = spike_sim->procs[0]->get_state()->last_inst_priv;
    spike_tunnel->spike_last_inst_xlen = spike_sim->procs[0]->get_state()->last_inst_xlen;
    spike_tunnel->spike_last_inst_flen = spike_sim->procs[0]->get_state()->last_inst_flen;
    return true;
}

void spikeAdapter::setNpc(reg_t npc) {
    npc_ = npc;
    spike_sim->procs[0]->get_state()->pc = npc;
}

reg_t spikeAdapter::getNpcHook(reg_t spike_npc) {
    spike_npc_ = spike_npc;
    return spike_npc;
}

reg_t spikeAdapter::excptionHook(void* in, uint64_t pc){
    if (getPredictionMiss()) {
        auto fetch = (insn_fetch_t*) in;
        uint64_t npc = pc + fetch->insn.length();
        setNpc(npc);
        return -1;
    }

    return 0;
}

void spikeAdapter::catchDataBeforeWriteHook(addr_t addr, reg_t data, size_t len) {
//    if (spike_sim->get_tohost_addr() == addr ||
//        spike_sim->get_tohost_addr() + 0x4 == addr ||
//        spike_sim->get_fromhost_addr() == addr) {
//        return;
//    }

    uint64_t asset_cmp_data = 0;

    if (!getPredictionMiss()) {
        try {
        switch (len) {
            case (1):
                asset_cmp_data = spike_sim_fast->procs[0]->get_mmu()->load<uint8_t>(addr);
                break;
            case (2):
                asset_cmp_data = spike_sim_fast->procs[0]->get_mmu()->load<uint16_t>(addr);
                break;
            case (4):
                asset_cmp_data = spike_sim_fast->procs[0]->get_mmu()->load<uint32_t>(addr);
                break;
            case (8):
                asset_cmp_data = spike_sim_fast->procs[0]->get_mmu()->load<uint64_t>(addr);
                break;
            default:
                assert(false);
        }
        } catch(...) {
            asset_cmp_data = data;
        }
        sparta_assert(asset_cmp_data == data, "pc is:  " << spike_sim->procs[0]->get_state()->pc <<
                                              ", addr is: " << addr <<
                                              ", data is: " << data <<
                                              ", len is: " << len << "\n" <<
                                              "real data is: " << asset_cmp_data
                                              );
    } else {
        if (memory_backup_.IsEmpty()) {
            return;
        }

        try {
            uint64_t bytes = 0;
            switch (len) {
                case (1):
                    bytes = spike_sim->procs[0]->get_mmu()->load<uint8_t>(addr);
                    break;
                case (2):
                    bytes = spike_sim->procs[0]->get_mmu()->load<uint16_t>(addr);
                    break;
                case (4):
                    bytes = spike_sim->procs[0]->get_mmu()->load<uint32_t>(addr);
                    break;
                case (8):
                    bytes = spike_sim->procs[0]->get_mmu()->load<uint64_t>(addr);
                    break;
                default:
                    assert(false);
            }
            memory_backup_.Push(MemoryEntry{addr, bytes, len});
        } catch(...) {
            return;
        }
    }
}

bool spikeAdapter::getCsrHook(int which, reg_t val) {
    if (getPredictionMiss()) {
        return false;
    }
    return true;
}

void spikeAdapter::MakeBackup() {
    memory_backup_.MakeBackupEntry();
    MakeRegBackup_();
}

void spikeAdapter::RollBack() {
    MemoryRollBack_();

    CsrRollBack_();

    RegRollBack_();
}

void spikeAdapter::BranchResolve(bool is_miss_prediction) {
    if (is_miss_prediction) {
        RollBack();
    } else {
        memory_backup_.Pop();
        reg_backup_.pop();
    }
}

void spikeAdapter::spikeRunStart(){
    spike_sim->start();
    spike_sim_fast->start();

    fromhost_callback = [this](uint64_t x) { fromhost_queue.push(x); };
    fromhost_callback_fast = [this](uint64_t x) {};
}

int spikeAdapter::spikeRunEnd_(){
    spike_sim->stop();
    spike_sim_fast->stop();
    is_done = true;
    return spike_sim->exit_code();
}

static void bad_address(const std::string& situation, reg_t addr)
{
  std::cerr << "Access exception occurred while " << situation << ":\n";
  std::cerr << "Memory address 0x" << std::hex << addr << " is invalid\n";
  exit(-1);
}

int spikeAdapter::spikeStep(uint32_t n){
    // sim_t::INTERLEAVE = n;
    uint64_t tohost;
    for (int i=0; i<n; i++){
      if (spike_sim->get_tohost_addr() == 0) {
            spike_sim->idle();
      }else{
          try {
                if ((tohost = spike_sim->from_target(spike_sim->mem.read_uint64(spike_sim->get_tohost_addr()))) != 0 &&
                    !getPredictionMiss()) {
                    spike_sim->mem.write_uint64(spike_sim->get_tohost_addr(), target_endian<uint64_t>::zero);
                    spike_sim_fast->mem.write_uint64(spike_sim->get_tohost_addr(), target_endian<uint64_t>::zero);
                }
          } catch (mem_trap_t& t) {
                bad_address("accessing tohost", t.get_tval());
          }

          try {
            if (tohost != 0 && !getPredictionMiss()) {
                command_t cmd(spike_sim->mem, tohost, fromhost_callback);
                command_t cmd_fast(spike_sim_fast->mem, tohost, fromhost_callback_fast);
                spike_sim->device_list.handle_command(cmd);
                spike_sim_fast->device_list.handle_command(cmd_fast);
            } else {
                if (!getPredictionMiss()) {
                    spike_sim_fast->idle();
                }
                spike_sim->idle();
                if (!getPredictionMiss()) {
                    assert(spike_sim->procs[0]->get_state()->pc ==
                           spike_sim_fast->procs[0]->get_state()->pc);

                    sparta_assert(RegEqual_(spike_sim->procs[0]->get_state(), spike_sim_fast->procs[0]->get_state()),
                                  "pc is: " << spike_sim->procs[0]->get_state()->pc << "\n" <<
                                  getScalarRegStates_(spike_sim->procs[0]->get_state()) <<
                                  getScalarRegStates_(spike_sim_fast->procs[0]->get_state()));
                }
            }

            spike_sim->device_list.tick();
          } catch (mem_trap_t& t) {
            std::stringstream tohost_hex;
            tohost_hex << std::hex << tohost;
            bad_address("host was accessing memory on behalf of target (tohost = 0x" + tohost_hex.str() + ")", t.get_tval());
          }

          try {
            if (!fromhost_queue.empty() && !spike_sim->mem.read_uint64(spike_sim->get_fromhost_addr())) {
                spike_sim->mem.write_uint64(spike_sim->get_fromhost_addr(), spike_sim->to_target(fromhost_queue.front()));
                spike_sim_fast->mem.write_uint64(spike_sim->get_fromhost_addr(), spike_sim->to_target(fromhost_queue.front()));
                fromhost_queue.pop();
            }
          } catch (mem_trap_t& t) {
            bad_address("accessing fromhost", t.get_tval());
          }
      }

      if (spike_sim->exitcode != 0){
        spikeRunEnd_();
        return -1;
      }
    }

    return 0;
}

void spikeAdapter::MakeRegBackup_() {
    state_t* state_tmp = spike_sim->procs[0]->get_state();
    RegEntry reg_entry;
    reg_entry.XPR               = state_tmp->XPR;
    reg_entry.FPR               = state_tmp->FPR;
    reg_entry.prv               = state_tmp->prv;
    reg_entry.prev_prv          = state_tmp->prev_prv;
    reg_entry.prv_changed       = state_tmp->prv_changed;
    reg_entry.v_changed         = state_tmp->v_changed;
    reg_entry.v                 = state_tmp->v;
    reg_entry.prev_v            = state_tmp->prev_v;
    reg_entry.debug_mode        = state_tmp->debug_mode;
    reg_entry.serialized        = state_tmp->serialized;
    reg_entry.log_reg_write     = state_tmp->log_reg_write;
    reg_entry.log_mem_read      = state_tmp->log_mem_read;
    reg_entry.log_mem_write     = state_tmp->log_mem_write;
    reg_entry.last_inst_priv    = state_tmp->last_inst_priv;
    reg_entry.last_inst_xlen    = state_tmp->last_inst_xlen;
    reg_entry.last_inst_flen    = state_tmp->last_inst_flen;
    reg_backup_.push(reg_entry);
}

std::string spikeAdapter::getScalarRegStates_(const state_t* state) {
    std::string reg_states = "";
    for (int i = 0; i < 32; i++) {
        reg_states += "x" + std::to_string(i) + " = " + std::to_string(state->XPR[i]) + "\n";
    }
    return reg_states;
}

void spikeAdapter::RegRollBack_() {
    state_t* state_tmp = spike_sim->procs[0]->get_state();
    state_t* state_fast_tmp = spike_sim_fast->procs[0]->get_state();
    RegEntry reg_entry = reg_backup_.front();
    state_tmp->pc = npc_;
    state_tmp->XPR              = reg_entry.XPR           ;
    state_tmp->FPR              = reg_entry.FPR           ;
    state_tmp->prv              = reg_entry.prv           ;
    state_tmp->prev_prv         = reg_entry.prev_prv      ;
    state_tmp->prv_changed      = reg_entry.prv_changed   ;
    state_tmp->v_changed        = reg_entry.v_changed     ;
    state_tmp->v                = reg_entry.v             ;
    state_tmp->prev_v           = reg_entry.prev_v        ;
    state_tmp->debug_mode       = reg_entry.debug_mode    ;
    state_tmp->serialized       = reg_entry.serialized    ;
    state_tmp->log_reg_write    = reg_entry.log_reg_write ;
    state_tmp->log_mem_read     = reg_entry.log_mem_read  ;
    state_tmp->log_mem_write    = reg_entry.log_mem_write ;
    state_tmp->last_inst_priv   = reg_entry.last_inst_priv;
    state_tmp->last_inst_xlen   = reg_entry.last_inst_xlen;
    state_tmp->last_inst_flen   = reg_entry.last_inst_flen;
    assert(RegEqual_(state_tmp, state_fast_tmp));
    while (!reg_backup_.empty()) {
        reg_backup_.pop();
    }
}

bool spikeAdapter::RegEqual_(state_t* state_1, state_t* state_2) {
    bool reg_equal = true;
    for (int i = 0; i < 32; i++) { // TODO: ERROR when register is not equal to 32
        if (state_1->XPR[i] != state_2->XPR[i] ||
            state_1->FPR[i].v[0] != state_2->FPR[i].v[0] ||
            state_1->FPR[i].v[1] != state_2->FPR[i].v[1]) {
            reg_equal = false;
            break;
        }
    }

    bool prv_equal          =       state_1->prv              == state_2->prv           ;
    bool prev_prv_equal     =       state_1->prev_prv         == state_2->prev_prv      ;
    bool prv_changed_equal  =       state_1->prv_changed      == state_2->prv_changed   ;
    bool v_changed_equal    =       state_1->v_changed        == state_2->v_changed     ;
    bool v_equal            =       state_1->v                == state_2->v             ;
    bool prev_v_equal       =       state_1->prev_v           == state_2->prev_v        ;
    bool debug_mode_equal   =       state_1->debug_mode       == state_2->debug_mode    ;
    bool serialized_equal   =       state_1->serialized       == state_2->serialized    ;

    return  prv_equal && prev_prv_equal && prv_changed_equal && v_changed_equal && v_equal &&
            prev_v_equal && debug_mode_equal && serialized_equal && reg_equal;
}

void spikeAdapter::MemoryRollBack_() {
    while(!memory_backup_.IsEmpty()) {
        MemoryEntry memory_entry = memory_backup_.GetBackupEntry();
        if (memory_entry.addr == 0) {
            continue;
        }
        uint64_t addr_offset = memory_entry.addr % 8;
        memory_entry.data = memory_entry.data << addr_offset * 8;
        uint64_t addr_align  = memory_entry.addr - addr_offset;
        auto memory_old_entry_itr = memory_clean_backup_map_.find(addr_align);
        if (memory_old_entry_itr != memory_clean_backup_map_.end()) {
            uint64_t bit_mask_new_entry = (UINT64_MAX >> (64 - 8 * memory_entry.len)) << addr_offset * 8;
            uint64_t bit_mask_overlap = bit_mask_new_entry & memory_old_entry_itr->second.bit_mask;
            uint64_t bit_mask_new_entry_exclusive = ~bit_mask_overlap & bit_mask_new_entry;
            uint64_t bit_mask_old_entry_exclusive = ~bit_mask_overlap & memory_old_entry_itr->second.bit_mask;
            uint64_t bit_mask_final = bit_mask_new_entry | memory_old_entry_itr->second.bit_mask;
            memory_old_entry_itr->second.data =
                    (bit_mask_old_entry_exclusive & memory_old_entry_itr->second.data) |
                    (bit_mask_new_entry_exclusive & memory_entry.data) |
                    (bit_mask_overlap & memory_entry.data);
            memory_old_entry_itr->second.bit_mask = bit_mask_final;
            assert(memory_old_entry_itr->second.len == 8);
        } else {
            memory_entry.bit_mask = (UINT64_MAX >> (64 - 8 * memory_entry.len)) << addr_offset * 8;
            memory_entry.len = 8;
            memory_entry.addr = addr_align;
            memory_clean_backup_map_[addr_align] = memory_entry;
        }
    }

    for (auto memory_entry_pair: memory_clean_backup_map_) {
        auto memory_entry = memory_entry_pair.second;
        target_addr_ = memory_entry.addr & ~(spike_sim->chunk_align() - 1);
        uint64_t mem_data = 0;
        mem_data = spike_sim_fast->procs[0]->get_mmu()->load<uint64_t>(memory_entry.addr);
        assert((mem_data & memory_entry.bit_mask) ==
               memory_entry.data);
        for (int i = 0; i < 8; i++) {
            uint64_t data = memory_entry.data >> i * 8 & 0xFF;
            if ((memory_entry.bit_mask >> i * 8 & 0xFF) == 0xFF) {
                spike_sim->procs[0]->get_mmu()->store<uint8_t>(memory_entry.addr + i, data);
            }
        }
    }
    memory_clean_backup_map_.clear();
}

void spikeAdapter::CsrRollBack_() {
    spike_sim->procs[0]->put_csr(0xb00, spike_sim_fast->procs[0]->get_csr(0xb00) + 1);
    spike_sim->procs[0]->put_csr(0xb02, spike_sim_fast->procs[0]->get_csr(0xb02) + 1);
}