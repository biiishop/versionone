# TODO (Khangaroo): Make this process a lot less hacky (no, export did not work)
# See MakefileNSO

.PHONY: all clean starlight send

SMOVER ?= 100
IP ?= 192.168.0.185 # Put Switch IP Here (for make send)
USERNAME ?= abcd # Put username here (for make send)
PASS ?= 12345 # put password here (for make send)
SERVERIP ?= 0.0.0.0 # put debug logger server IP here

PROJNAME ?= StarlightBase

all: starlight

starlight:
	$(MAKE) all -f MakefileNSO SMOVER=$(SMOVER) SERVERIP=${SERVERIP}
	$(MAKE) starlight_patch_$(SMOVER)/*.ips
	
	mkdir -p starlight_patch_$(SMOVER)/atmosphere/exefs_patches/$(PROJNAME)/
	mkdir -p starlight_patch_$(SMOVER)/atmosphere/contents/0100000000010000/exefs/
	
	mv starlight_patch_$(SMOVER)/3CA12DFAAF9C82DA064D1698DF79CDA1.ips starlight_patch_$(SMOVER)/atmosphere/exefs_patches/$(PROJNAME)/3CA12DFAAF9C82DA064D1698DF79CDA1.ips
	mv $(shell basename $(CURDIR))$(SMOVER).elf starlight_patch_$(SMOVER)/subsdk1.elf
	mv $(shell basename $(CURDIR))$(SMOVER).nso starlight_patch_$(SMOVER)/atmosphere/contents/0100000000010000/exefs/subsdk1

starlight_patch_$(SMOVER)/*.ips: patches/*.slpatch patches/configs/$(SMOVER).config patches/maps/$(SMOVER)/*.map \
								build$(SMOVER)/$(shell basename $(CURDIR))$(SMOVER).map scripts/genPatch.py
	@rm -f starlight_patch_$(SMOVER)/*.ips
	python3 scripts/genPatch.py $(SMOVER)

# NOTE: Make sure you configure sendPatch.py to use your login for ftp servers!
send: all
	python3 scripts/sendPatch.py $(IP) $(PROJNAME) $(USERNAME) $(PASS)

log: all
	python3 scripts/tcpServer.py $(SERVERIP)

sendlog: all
	python3 scripts/sendPatch.py $(IP) $(PROJNAME) $(USERNAME) $(PASS)
	python3 scripts/tcpServer.py $(SERVERIP)

clean:
	$(MAKE) clean -f MakefileNSO
	@rm -fr starlight_patch_*