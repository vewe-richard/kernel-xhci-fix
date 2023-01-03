# Kernel customization

# Root linux-stable/ tag: v5.11-rc7
  build: make -j8
         make INSTALL_MOD_PATH=../modules/ modules_install
         tar cvf modules.tar.gz modules/lib/modules/
	 scp -P 5555 -r modules.tar.gz  richard@localhost:/tmp/
	 sudo tar xf /tmp/modules.tar.gz -C /lib/modules/


    
