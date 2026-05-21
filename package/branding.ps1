$PackageName = "Euro-Office Desktop Editors"
$BuildDir = "build"
$DesktopDir = "DesktopEditors"

Function BrandingAdvInstConfig {
    return `
        "DelFolder CUSTOM_PATH", `
        "SetProductCode -langid 1029 -guid $ProductCode", `
        "SetProductCode -langid 1031 -guid $ProductCode", `
        "SetProductCode -langid 1033 -guid $ProductCode", `
        "SetProductCode -langid 1036 -guid $ProductCode", `
        "SetProductCode -langid 1041 -guid $ProductCode", `
        "SetProductCode -langid 1046 -guid $ProductCode", `
        "SetProductCode -langid 1049 -guid $ProductCode", `
        "SetProductCode -langid 1060 -guid $ProductCode", `
        "SetProductCode -langid 2070 -guid $ProductCode", `
        "SetProductCode -langid 3082 -guid $ProductCode", `
        "SetProperty FORMS=1"
}
