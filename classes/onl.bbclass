# based on poky/meta/classes/devupstream.bbclass
#
# Needed as defining a kernel BBCLASSEXTEND will trigger the inclusion of an
# appropriately named bbclass.

python onl_virtclass_handler () {
    # Do nothing if this is inherited, as it's for BBCLASSEXTEND
    if "onl" not in (d.getVar('BBCLASSEXTEND') or ""):
        bb.error("Don't inherit onl, use BBCLASSEXTEND")
        return

    variant = d.getVar("BBEXTENDVARIANT")
    if variant not in ("target", "native"):
        bb.error("Unsupported variant %s. Pass the variant when using onl, for example onl:target" % variant)
        return

    # Develpment releases are never preferred by default
    d.setVar("DEFAULT_PREFERENCE", "-1")

    src_uri = d.getVar("SRC_URI:class-onl") or d.getVar("SRC_URI")
    uri = bb.fetch2.URI(src_uri.split()[0])

    if uri.scheme == "git" and not d.getVar("S:class-onl"):
        d.setVar("S", "${WORKDIR}/git")

    # Modify the PV if the recipe hasn't already overridden it
    pv = d.getVar("PV")
    proto_marker = "+" + uri.scheme
    if proto_marker not in pv and not d.getVar("PV:class-onl"):
        d.setVar("PV", pv + proto_marker + "${SRCPV}")

    if variant == "native":
        pn = d.getVar("PN")
        d.setVar("PN", "%s-native" % (pn))
        fn = d.getVar("FILE")
        bb.parse.BBHandler.inherit("native", fn, 0, d)

    d.appendVar("CLASSOVERRIDE", ":class-onl")
}

addhandler onl_virtclass_handler
onl_virtclass_handler[eventmask] = "bb.event.RecipePreFinalise"
