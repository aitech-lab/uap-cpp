namespace metrics_v01 {
    
    // BLOCK 0
    enum block_0 {
        m_math,

        m_connectEnd,
        m_connectStart,
        m_domComplete,
        m_domContentLoadedEventEnd,
        m_domContentLoadedEventStart,
        m_domInteractive,
        m_domLoading,
        m_domainLookupEnd,
        m_domainLookupStart,
        m_fetchStart,
        m_loadEventEnd,
        m_loadEventStart,
        m_navigationStart,
        m_redirectEnd,
        m_redirectStart,
        m_requestStart,
        m_responseEnd,
        m_responseStart,
        m_secureConnectionStart,
        m_unloadEventEnd,
        m_unloadEventStart,

        m_f_default,
        m_f_apple,
        m_f_serif,
        m_f_sans,
        m_f_mono,
        m_f_min,
        m_f_system,

        m_audio
    };


    // BLOCK 1
    enum block_1 {
        m_sessionStorage,
        m_localStorage,
        m_indexedDB,
        m_openDatabase,
        m_cookiesEnabled,
        m_forcedColors,
        m_reducedMotion,
        m_deviceMemory,
        m_colorDepth,
        m_hardwareConcurrency,
        m_monochrome,
        m_incognito,

        m_maxTouchPoints,
        m_touchEvent,
        m_touchStart,

        m_screenResolution_w,
        m_screenResolution_h,

        m_screenFrame_0,
        m_screenFrame_1,
        m_screenFrame_2,
        m_screenFrame_3
    };


    // BLOCK 2
    enum block_2 {
        m_timezone,
        m_platform,
        m_vendor,
        m_colorGamut,
        m_osCpu,

        m_canvas_geom,
        m_canvas_text,
        m_plugins,
        m_vendorFlavors,
        m_languages
    };

    /// BLOCK 3 fonts
    // enum block_3 {
    //     font_0, ... ,fonts_32
    // }
                    
    int fp_unstable_block_0[] = {
        m_f_default,
        m_f_apple,
        m_f_serif,
        m_f_sans,
        m_f_mono,
        m_f_min,
        m_f_system,
        m_audio
    };

    int fp_unstable_block_2[] = {
        m_canvas_geom,
        m_canvas_text,
        m_plugins,
        m_vendorFlavors,
        m_languages
    };
};


namespace metrics_v02 {

    // BLOCK 0
    enum block_0 {
        m_math,

        m_f_default,
        m_f_apple,
        m_f_serif,
        m_f_sans,
        m_f_mono,
        m_f_min,
        m_f_system,

        m_audio
    };

    // BLOCK 1
    enum block_1 {
        m_sessionStorage,
        m_localStorage,
        m_indexedDB,
        m_openDatabase,
        m_cookiesEnabled,
        m_forcedColors,
        m_reducedMotion,
        m_deviceMemory,
        m_colorDepth,
        m_hardwareConcurrency,
        m_monochrome,
        m_incognito,

        m_maxTouchPoints,
        m_touchEvent,
        m_touchStart,

        m_screenResolution_w,
        m_screenResolution_h,

        m_screenFrame_0,
        m_screenFrame_1,
        m_screenFrame_2,
        m_screenFrame_3
    };


    // BLOCK 2
    enum block_2 {
        m_timezone,
        m_platform,
        m_vendor,
        m_colorGamut,
        m_osCpu,

        m_canvas_geom,
        m_canvas_text,
        m_plugins,
        m_vendorFlavors,
        m_languages
    };

    /// BLOCK 3 fonts
    // enum block_3 {
    //     font_0, ... ,fonts_32
    // }
                    
    int fp_unstable_block_0[] = {
        m_f_default,
        m_f_apple,
        m_f_serif,
        m_f_sans,
        m_f_mono,
        m_f_min,
        m_f_system,
        m_audio
    };

    int fp_unstable_block_2[] = {
        m_canvas_geom,
        m_canvas_text,
        m_plugins,
        m_vendorFlavors,
        m_languages
    };
};