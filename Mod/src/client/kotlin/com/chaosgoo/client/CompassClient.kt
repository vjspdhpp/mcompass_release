package com.chaosgoo.client

import kotlinx.coroutines.*
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import net.fabricmc.api.ClientModInitializer
import net.fabricmc.fabric.api.client.event.lifecycle.v1.ClientTickEvents
import net.minecraft.text.Text
import okhttp3.*
import retrofit2.Retrofit
import retrofit2.http.GET
import retrofit2.http.POST
import retrofit2.http.Query

interface CompassApiService {
    @GET("ip")
    suspend fun getIP(): String

    @POST("setAzimuth")
    suspend fun setAzimuth(@Query("azimuth") azimuth: Float)
}

class CompassClient : ClientModInitializer {
    private var lastAzimuth = 0f
    private val retrofit by lazy {
        Retrofit.Builder()
            .baseUrl("http://esp32.local")
            .client(OkHttpClient())
            .build()
    }
    private val apiService by lazy { retrofit.create(CompassApiService::class.java) }
    private val mutex = Mutex()

    private val dispatcher = Dispatchers.IO.limitedParallelism(8)
    private val scope = CoroutineScope(SupervisorJob() + dispatcher)

    override fun onInitializeClient() {
        ClientTickEvents.END_CLIENT_TICK.register(
            ClientTickEvents.EndTick { client ->
                client.player?.let { player ->
                    val pos = player.pos
                    val rawAzimuth = player.yaw % 360
                    val azimuth = if (rawAzimuth < 0) rawAzimuth + 360 else rawAzimuth
                    player.sendMessage(
                        Text.of(
                            "Pos:(${"%.2f".format(pos.x)},${"%.2f".format(pos.y)},${"%.2f".format(pos.z)}), Yaw:${
                                "%.2f".format(
                                    azimuth
                                )
                            }"
                        ),
                        true
                    )
                    if (lastAzimuth != azimuth) {
                        setAzimuth(lastAzimuth)
                        lastAzimuth = azimuth
                    }
                }
            }
        )
    }

    private fun setAzimuth(azimuth: Float) {
        if (mutex.isLocked) return
        scope.launch {
            mutex.withLock {
                apiService.setAzimuth(azimuth)
            }
        }
    }
}
