/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package MessageDeserializer;

import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonDeserializer;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.Gson;
import java.lang.reflect.Type;
import sg.smart.mit.simmobility4android.message.TimeMessage;

/**
 *
 * @author vahid
 */
public class TimeMessageDeserializer implements JsonDeserializer<TimeMessage> {
    @Override
    public TimeMessage deserialize(JsonElement je, Type type, JsonDeserializationContext jdc)
    {
      JsonObject jo = je.getAsJsonObject().getAsJsonObject("TimeMessage"); 
        Gson g = new Gson();
        return g.fromJson(jo, TimeMessage.class); 
      
    }
}
